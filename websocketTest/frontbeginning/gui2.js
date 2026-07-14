console.log('test');

const url = 'http://localhost:7681';
const maxR0ws = 50;
const renderMilliseconds = 250;
//might not be best syntax to get/store the data

//state
const buffer = [];

//vars
const packPerSec = document.getElementById('packetsPerSecond');
const packProcessed = document.getElementById('packetsProcessed');
const rows = document.getElementById('rows');

let packetsProcessed = 0;
let packetsPerSecond = 0;

setInterval(() => {
    packPerSec.textContent = packetsPerSecond;
    packetsPerSecond = 0;
}, 1000);

function filters(){
    
}

function addRow(packet){
    const tr = document.createElement('tr');
    const time = new Date().toLocaleString();
    tr.innerHTML = 
        `<td>${time}</td>
        <td>${packet.source_ip}</td>
        <td>${packet.dest_ip}</td>
        <td>${packet.source_port}</td>
        <td>${packet.dest_port}</td>
        <td>${packet.protocol}</td>
        <td>${packet.bytes}</td>`;
    rows.prepend(tr);
    while(rows.childElementCount > maxR0ws){
        rows.removeChild(rows.lastChild);
    }
}

function main(){
    const ws = new WebSocket("ws://localhost:7681", "prot1");

    ws.onmessage = function(e){
        let packeter;
        try{
            packeter = JSON.parse(packet.data); 
        }
        catch(e){
            console.log('error parsing packet data');
        }
        if(packeter.type == "sidebar"){
            filters(packater);
            return;
        }
        addRow(packeter);
        packetsProcessed++;
        packProcessed.textContent = packetsProcessed;
        packetsPerSecond++;
    };

    ws.onopen = function(e){
        console.log('ws opened');
    }
    ws.onclose = function(e){
        console.log('ws closed');
    };
    ws.onerror = function(e){
        console.log('ws error');
    }

}
document.addEventListener('DOMContentLoaded', main);



//const data = JSON.parse(localStorage.getItem('packetData'));
//i dont like that it's local storage
/* const form = document.getElementById('inputForm');
if(form){
    form.addEventListener('submit', function(e){
        e.preventDefault();
        const data = Object.fromEntries(new FormData(this));
        localStorage.setItem('packetFilters', JSON.stringify(data));
        window.location.href = 'gui.html';
    })
} */


//fix up for the input page form
/* document.getElementById('packetForm').addEventListener('submit', function(e) {
    e.preventDefault();
    const data = Object.fromEntries(new FormData(this));
    const timestamp = new Date().toISOString();
    const tbody = document.getElementById('rows');
    const empty = tbody.querySelector('.empty');
    if (empty) empty.parentElement.remove();
    const tr = document.createElement('tr');
    tr.innerHTML = `<td>${timestamp}</td><td>${data.srcIp}</td><td>${data.dstIp}</td><td>${data.srcPort}</td><td>${data.dstPort}</td><td>${data.protocol}</td><td>${data.bytes}</td>`;
    tbody.appendChild(tr);
});
 */








