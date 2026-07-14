#include <libwebsockets.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

static int interrupted;

//one per msg
struct msg{
    void *payload; //malloced
    size_t len;
};

//one per connected client
struct per_session_data_prot1{ 
    struct per_session_data_prot1 *pss_list; //linked list of all sessions
    struct lws *wsi; //websocket instance
    uint32_t tail;
};

//One for vhost
struct per_vhost_data_prot1{
    struct lws_context *context;
    struct lws_vhost *vhost;
    const struct lws_protocols *protocol;
    struct per_session_data_prot1 *pss_list; 
    struct lws_ring *ring; //ring buffer for messages instead of struct msg
};

static void prot1_destroy_msg(void *_msg){ 
    struct msg *msg = _msg;
    free(msg->payload);
    msg->payload = NULL;
    msg->len = 0;
}

static void sender(struct per_vhost_data_prot1 *vhd, char *msg, size_t len){
    struct msg a_msg;
    if(!lws_ring_get_count_free_elements(vhd->ring)){
        lwsl_user("ring full, dropping\n");   // slow/absent client: drop newest
        return;
    }
    a_msg.len = len;
    a_msg.payload = malloc(LWS_PRE + len);
    if(!a_msg.payload){ 
        lwsl_user("OOM: dropping\n"); 
        return; 
    }
    memcpy((char *)a_msg.payload + LWS_PRE, msg, len);

    if(!lws_ring_insert(vhd->ring, &a_msg, 1)){
        prot1_destroy_msg(&a_msg);
        lwsl_user("dropping!\n");
        return;
    }
    lws_start_foreach_llp(struct per_session_data_prot1 **, ppss, vhd->pss_list){
        lws_callback_on_writable((*ppss)->wsi);
    } lws_end_foreach_llp(ppss, pss_list);
}

static int callbackFunc(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len){
    struct per_session_data_prot1 *pss = (struct per_session_data_prot1 *)user;
    struct per_vhost_data_prot1 *vhd = (struct per_vhost_data_prot1 *)lws_protocol_vh_priv_get(lws_get_vhost(wsi), lws_get_protocol(wsi));
    int m;
    lws_sock_file_fd_type u;

    switch(reason){
        //for fun really the first case
        //check origin to make sure only the one i want is connecting
        //need curly brackets cuz delceration cant come after case label
        case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:{
            char origin[128];
            int n = lws_hdr_copy(wsi, origin, sizeof(origin), WSI_TOKEN_ORIGIN);
            if(n<=0){
                lwsl_user("rejecting not origin\n");
                return -1;
            }
            if(strcmp(origin, "http://localhost:7681") != 0){
                lwsl_user("rejecting origin %s\n", origin);
                return -1;
            }
            break;
        }
        case LWS_CALLBACK_PROTOCOL_INIT: //intialization
            vhd = lws_protocol_vh_priv_zalloc(lws_get_vhost(wsi), lws_get_protocol(wsi), sizeof(struct per_vhost_data_prot1));
            if(!vhd){
                return 1;
            }
            vhd->context = lws_get_context(wsi);
            vhd->protocol = lws_get_protocol(wsi);
            vhd->vhost = lws_get_vhost(wsi);
            
            vhd->ring = lws_ring_create(sizeof(struct msg), 1024, prot1_destroy_msg); //up to 1024 msgs in ring buffer
            if(!vhd->ring){
                lwsl_user("Failed to create ring\n");
                return 1;
            }

            u.filefd = (lws_filefd_type)(long long)STDIN_FILENO;
            if(!lws_adopt_descriptor_vhost(lws_get_vhost(wsi), LWS_ADOPT_RAW_FILE_DESC, u, "prot1", NULL)){
                lwsl_err("Failed to adopt stdin\n");
                return 1;
            }
            break;
        case LWS_CALLBACK_PROTOCOL_DESTROY:
            lws_ring_destroy(vhd->ring);
            break;
        case LWS_CALLBACK_ESTABLISHED: //new connection/client established
            lws_ll_fwd_insert(pss, pss_list, vhd->pss_list);
            pss->wsi = wsi;
            pss->tail = lws_ring_get_oldest_tail(vhd->ring);
            break;
        case LWS_CALLBACK_CLOSED:
            lws_ll_fwd_remove(struct per_session_data_prot1, pss_list, pss, vhd->pss_list);
            break;
        case LWS_CALLBACK_RAW_CLOSE_FILE:
            lwsl_user("stdin closed\n");
            interrupted = 1;
            break;
        case LWS_CALLBACK_SERVER_WRITEABLE:
            {const struct msg *a_msg = lws_ring_get_element(vhd->ring, &pss->tail);
            if(!a_msg) //funny syntax messes with my muscle memory
                break;
            m = lws_write(wsi, (unsigned char *)a_msg->payload + LWS_PRE, a_msg->len, LWS_WRITE_TEXT);
            if(m < (int)a_msg->len){
                lwsl_err("ERROR %d writing to ws socket\n", m);
                return -1;
            }
            lws_ring_consume_and_update_oldest_tail(vhd->ring, struct per_session_data_prot1, &pss->tail, 1, vhd->pss_list, tail, pss_list);
            if(lws_ring_get_element(vhd->ring, &pss->tail)){
                lws_callback_on_writable(wsi);
            }
            }
            break;
        case LWS_CALLBACK_RECEIVE: //technically dont need it for my use case for now
            /* if(vhd->a_msg.payload){ //make it better with ringn examlpe with queue
                //basically if client sends msg, we free old msg and store new one
                prot1_destroy_msg(&vhd->a_msg);
            }
            vhd->a_msg.len = len;
            vhd->a_msg.payload = malloc(LWS_PRE + len);
            if(!vhd->a_msg.payload){
                lwsl_user("OOM: dropping\n"); //literally a print statement - lvls and all but yeah
                break;
            }
            memcpy((char*)vhd->a_msg.payload + LWS_PRE, in, len);
            vhd->current++;
            lws_start_foreach_llp(struct per_session_data_prot1 **, ppss, vhd->pss_list){
                lws_callback_on_writable((*ppss)->wsi);
            } lws_end_foreach_llp(ppss, pss_list);
            //loops through each client and calls callback on each one */
            break;
        case LWS_CALLBACK_RAW_RX_FILE:
            {static char buf[65536]; //overarching buffer
            static size_t acc_len = 0;
            int r;
            r = (int)read(lws_get_socket_fd(wsi), buf+acc_len, sizeof(buf)-acc_len); //overkill size
            //STDIN_FILENO
            if(r==0){
                lwsl_user("EOF: exiting\n");
                interrupted = 1;
                break;
            }
            if(r<0){
                lwsl_user("Failed read\n");
                break;
            }
            if(acc_len + (size_t)r > sizeof(buf)){
                acc_len = 0; //reset buffer if overflow
                //memset(buf, 0, 65536); 
            }
            acc_len+=r;
            
            size_t start = 0;
            for(size_t i = 0; i<acc_len; i++){
                if(buf[i] == '\n'){
                    if(i>start){
                        sender(vhd, buf+start, i-start);
                    }
                    start = i+1;
                }
            }
            acc_len -= start;
            memmove(buf, buf+start, acc_len);
            }
            break;
        default:
            break;
    }
    return 0;
}

/* #define CALLBACKPROTOCOL1 \
{ \
    "prot1", \
    callbackFunc, \
    sizeof(struct per_session_data_prot1), \
    65536, \
    0, NULL, 0 \
} */
// max size? other stuff in here gotta add
//gonna hve to make it a lot bigger or reasmmble it
//name has to match exact




