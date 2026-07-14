console.log('test');

const url = 'http://localhost:8080';
const maxR0ws = 30;
const renderMilliseconds = 250;
//might not be best syntax to get/store the data

//state
const buffer = [];

function new_ws(){ //hardcoded rn
    return new WebSocket("ws://localhost:7681", "prot1");
}

const ws = nen_ws();
ws.onmessage = function(e){
    buf+=data;
    let lines = buf.split('\n');
    buf = lines.pop();
    lines.forEach(function(line){
        if(!line) return;
        let jsoned = JSON.parse(line);
        
    });
};



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








