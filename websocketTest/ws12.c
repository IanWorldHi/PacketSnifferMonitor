#include <libwebsockets.h>

//one per msg
struct msg{
    void *payload; //malloced
    size_t len;
};

//one per connected client
struct per_session_data_prot1{ 
    struct per_session_data_prot1 *pss_list; //linked list of all sessions
    struct lws *wsi; //websocket instance
    int last; //last msg sent num
}



callbackFunc(){

}

#define CALLBACKPROTOCOL1 \ 
{ \
    "name?" \
    callbackFunc, \
    sizeof(), \ 
}
// max size? other stuff in here gotta add




