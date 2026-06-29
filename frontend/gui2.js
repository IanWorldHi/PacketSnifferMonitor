console.log('test');

const buffer = [];
//might not be best syntax to get/store the data


const form = document.getElementById('inputForm');
if(form){
    form.addEventListener('submit', function(e){
        e.preventDefault();

    })
}


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








