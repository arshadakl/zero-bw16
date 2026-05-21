#pragma once

const char INDEX_HTML[] = R"rawhtml(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>zero-bw16</title>
<style>
:root{--bg:#0d1117;--s1:#161b22;--s2:#21262d;--bd:#30363d;--tx:#e6edf3;--muted:#8b949e;--acc:#58a6ff;--red:#f85149;--grn:#3fb950;--ylw:#d29922;}
*{box-sizing:border-box;margin:0;padding:0;}
body{background:var(--bg);color:var(--tx);font:14px/1.5 'Segoe UI',system-ui,sans-serif;min-height:100vh;}
body.light{--bg:#f6f8fa;--s1:#ffffff;--s2:#f6f8fa;--bd:#d0d7de;--tx:#1f2328;--muted:#656d76;--acc:#0969da;--red:#cf222e;--grn:#1a7f37;--ylw:#9a6700;}
header{background:var(--s1);border-bottom:1px solid var(--bd);padding:12px 20px;display:flex;align-items:center;gap:12px;}
header h1{font-size:18px;font-weight:700;color:var(--acc);}header h1 span{color:var(--muted);font-size:12px;font-weight:400;margin-left:8px;}
.hdr-r{margin-left:auto;display:flex;align-items:center;gap:10px;}
.badge{display:inline-block;padding:2px 10px;border-radius:12px;font-size:12px;font-weight:600;background:var(--s2);border:1px solid var(--bd);}
.badge.idle{color:var(--muted)}.badge.scan{color:var(--ylw);border-color:var(--ylw)}.badge.atk{color:var(--red);border-color:var(--red);animation:pulse 1s infinite}
.badge.sniff{color:var(--grn);border-color:var(--grn)}
@keyframes pulse{0%,100%{opacity:1}50%{opacity:.5}}
.btn-icon{background:none;border:1px solid var(--bd);color:var(--tx);border-radius:6px;padding:4px 8px;cursor:pointer;font-size:14px;}
.btn-icon:hover{background:var(--s2);}
nav{background:var(--s1);border-bottom:1px solid var(--bd);padding:0 20px;display:flex;gap:0;overflow-x:auto;}
.tab{background:none;border:none;color:var(--muted);padding:12px 16px;cursor:pointer;font-size:13px;border-bottom:2px solid transparent;white-space:nowrap;}
.tab:hover{color:var(--tx)}.tab.active{color:var(--acc);border-bottom-color:var(--acc);}
.panel{display:none;padding:20px;max-width:1200px;}.panel.active{display:block;}
.card{background:var(--s1);border:1px solid var(--bd);border-radius:8px;padding:16px;margin-bottom:16px;}
.card-title{font-size:13px;font-weight:600;color:var(--muted);text-transform:uppercase;letter-spacing:.5px;margin-bottom:12px;}
.toolbar{display:flex;flex-wrap:wrap;gap:8px;margin-bottom:12px;align-items:center;}
.btn{padding:6px 14px;border-radius:6px;border:1px solid var(--bd);cursor:pointer;font-size:13px;font-weight:500;}
.btn-pri{background:var(--acc);color:#fff;border-color:var(--acc);}.btn-pri:hover{opacity:.85}
.btn-dng{background:var(--red);color:#fff;border-color:var(--red);}.btn-dng:hover{opacity:.85}
.btn-sec{background:var(--s2);color:var(--tx);}.btn-sec:hover{background:var(--bd);}
.btn-grn{background:var(--grn);color:#fff;border-color:var(--grn);}.btn-grn:hover{opacity:.85}
select,input[type=text],input[type=number],input[type=password],textarea{background:var(--s2);border:1px solid var(--bd);color:var(--tx);border-radius:6px;padding:5px 10px;font-size:13px;outline:none;}
select:focus,input:focus,textarea:focus{border-color:var(--acc);}
label{font-size:13px;color:var(--muted);}
.stats-row{display:flex;flex-wrap:wrap;gap:12px;margin-bottom:16px;}
.stat{background:var(--s2);border:1px solid var(--bd);border-radius:6px;padding:8px 14px;text-align:center;}
.stat-val{font-size:22px;font-weight:700;color:var(--acc);}.stat-lbl{font-size:11px;color:var(--muted);}
table{width:100%;border-collapse:collapse;font-size:13px;}
th{text-align:left;padding:8px 10px;color:var(--muted);border-bottom:1px solid var(--bd);font-weight:600;white-space:nowrap;}
td{padding:7px 10px;border-bottom:1px solid var(--s2);vertical-align:middle;}
tr:hover td{background:var(--s2);}
tr.expanded td{background:var(--s2);}
.sub-row td{background:var(--bg)!important;padding:0;}
.sub-inner{padding:8px 20px;}
input[type=checkbox]{width:16px;height:16px;cursor:pointer;}
.rssi-bar{display:inline-block;width:50px;height:8px;background:var(--bd);border-radius:4px;overflow:hidden;vertical-align:middle;}
.rssi-fill{height:100%;border-radius:4px;}
.enc-badge{display:inline-block;padding:1px 6px;border-radius:10px;font-size:11px;font-weight:600;}
.enc-open{background:#1f3a1f;color:var(--grn);}.enc-wpa{background:#1a2a3a;color:var(--acc);}.enc-wpa2{background:#1a2a3a;color:var(--acc);}.enc-wpa3{background:#1a1a3a;color:#bc8cff;}
.flag{display:inline-block;padding:1px 5px;border-radius:3px;font-size:10px;font-weight:700;margin-left:3px;}
.flag-wps{background:#3a2a00;color:var(--ylw);}.flag-pmf{background:#002040;color:var(--acc);}
.mode-select{min-width:160px;}
.log-container{background:var(--bg);border-radius:6px;padding:12px;max-height:400px;overflow-y:auto;font-family:monospace;font-size:12px;}
.log-info{color:var(--muted)}.log-warn{color:var(--ylw)}.log-error{color:var(--red)}.log-attack{color:#f78166}.log-sniff{color:var(--grn)}
.form-row{display:flex;align-items:center;gap:10px;margin-bottom:10px;flex-wrap:wrap;}
.form-row label{min-width:130px;}
.ssid-textarea{width:100%;height:120px;font-family:monospace;resize:vertical;}
.expand-btn{background:none;border:none;cursor:pointer;color:var(--muted);font-size:12px;padding:2px 6px;}
.expand-btn:hover{color:var(--tx);}
.ota-drop{border:2px dashed var(--bd);border-radius:8px;padding:30px;text-align:center;cursor:pointer;color:var(--muted);}
.ota-drop.drag{border-color:var(--acc);color:var(--acc);}
.ota-progress{display:none;margin-top:10px;}
progress{width:100%;height:8px;border-radius:4px;}
.mono{font-family:monospace;font-size:12px;}
.empty{color:var(--muted);font-style:italic;padding:20px;text-align:center;}
@media(max-width:600px){.form-row{flex-direction:column;align-items:flex-start;}.form-row label{min-width:auto;}}
</style>
</head>
<body>
<header>
  <h1>zero-bw16 <span id="fw-ver">v1.0.0</span></h1>
  <div class="hdr-r">
    <span class="badge idle" id="status-badge">IDLE</span>
    <div class="stat" style="padding:4px 10px;">
      <span id="pkt-count" style="color:var(--acc);font-weight:700;">0</span>
      <span style="color:var(--muted);font-size:11px;"> pkts</span>
    </div>
    <button class="btn-icon" onclick="toggleTheme()" title="Toggle theme">&#9680;</button>
  </div>
</header>
<nav>
  <button class="tab active" onclick="showTab('attack')">&#9888; Attack</button>
  <button class="tab" onclick="showTab('sniff')">&#128065; Sniff</button>
  <button class="tab" onclick="showTab('beacon')">&#128268; Beacon</button>
  <button class="tab" onclick="showTab('log')">&#128196; Log</button>
  <button class="tab" onclick="showTab('settings')">&#9881; Settings</button>
</nav>

<!-- ATTACK TAB -->
<div class="panel active" id="tab-attack">
  <div class="stats-row" id="atk-stats" style="display:none">
    <div class="stat"><div class="stat-val" id="s-deauth">0</div><div class="stat-lbl">Deauths</div></div>
    <div class="stat"><div class="stat-val" id="s-beacon">0</div><div class="stat-lbl">Beacons</div></div>
    <div class="stat"><div class="stat-val" id="s-auth">0</div><div class="stat-lbl">Auth Flood</div></div>
    <div class="stat"><div class="stat-val" id="s-assoc">0</div><div class="stat-lbl">Assoc Flood</div></div>
    <div class="stat"><div class="stat-val" id="s-elapsed">0s</div><div class="stat-lbl">Elapsed</div></div>
  </div>
  <div class="card">
    <div class="toolbar">
      <select class="mode-select" id="atk-mode">
        <option value="1">Deauth (Directed)</option>
        <option value="2">Deauth (Broadcast)</option>
        <option value="3">Auth Flood</option>
        <option value="4">Assoc Flood</option>
        <option value="6">Evil Twin AP</option>
      </select>
      <select id="atk-reason" title="Reason code">
        <option value="1">1 - Unspecified</option>
        <option value="2" selected>2 - Prev Auth Invalid</option>
        <option value="3">3 - Deauth Leaving</option>
        <option value="4">4 - Inactivity</option>
        <option value="5">5 - AP Busy</option>
        <option value="6">6 - Class 2 Frame</option>
        <option value="7">7 - Class 3 Frame</option>
        <option value="8">8 - Disassoc Leaving</option>
        <option value="9">9 - Not Authenticated</option>
      </select>
      <button class="btn btn-pri" onclick="launchAttack()">&#9889; Launch</button>
      <button class="btn btn-dng" onclick="stopAttack()">&#9724; Stop</button>
      <button class="btn btn-sec" onclick="doScan()">&#8635; Scan</button>
      <label style="margin-left:auto;"><input type="checkbox" id="sel-all" onchange="selAll(this.checked)"> All</label>
    </div>
    <div style="overflow-x:auto;">
    <table id="net-table">
      <thead><tr>
        <th></th><th>#</th><th>SSID</th><th>BSSID</th>
        <th>CH</th><th>RSSI</th><th>Enc</th><th>Flags</th><th>Clients</th><th>Sel</th>
      </tr></thead>
      <tbody id="net-tbody"><tr><td colspan="10" class="empty">Press Scan to discover networks</td></tr></tbody>
    </table>
    </div>
  </div>
</div>

<!-- SNIFF TAB -->
<div class="panel" id="tab-sniff">
  <div class="toolbar">
    <button class="btn btn-grn" onclick="startSniff()">&#9679; Start Sniff</button>
    <button class="btn btn-dng" onclick="stopSniff()">&#9724; Stop</button>
    <button class="btn btn-sec" onclick="clearProbes()">Clear Probes</button>
    <button class="btn btn-sec" onclick="clearClients()">Clear Clients</button>
  </div>
  <div class="card">
    <div class="card-title">Probe Requests <span id="probe-cnt" class="badge">0</span></div>
    <div style="overflow-x:auto;">
    <table id="probe-table">
      <thead><tr><th>Source MAC</th><th>SSID Probed</th><th>RSSI</th><th>First Seen</th><th>Count</th></tr></thead>
      <tbody id="probe-tbody"><tr><td colspan="5" class="empty">No probe requests captured</td></tr></tbody>
    </table>
    </div>
  </div>
  <div class="card">
    <div class="card-title">Client Map <span id="client-cnt" class="badge">0</span></div>
    <div style="overflow-x:auto;">
    <table id="client-table">
      <thead><tr><th>Client MAC</th><th>AP BSSID</th><th>SSID</th><th>Last Seen</th></tr></thead>
      <tbody id="client-tbody"><tr><td colspan="4" class="empty">No clients observed</td></tr></tbody>
    </table>
    </div>
  </div>
  <div class="card">
    <div class="card-title">PMKID Captures <span id="pmkid-cnt" class="badge">0</span></div>
    <div style="overflow-x:auto;">
    <table id="pmkid-table">
      <thead><tr><th>BSSID</th><th>SSID</th><th>Client</th><th>PMKID (hex)</th><th>Time</th></tr></thead>
      <tbody id="pmkid-tbody"><tr><td colspan="5" class="empty">No PMKIDs captured</td></tr></tbody>
    </table>
    </div>
  </div>
  <div class="card">
    <div class="card-title">Handshake Captures <span id="hs-cnt" class="badge">0</span></div>
    <div style="overflow-x:auto;">
    <table id="hs-table">
      <thead><tr><th>BSSID</th><th>Client</th><th>Msgs</th><th>MIC (hex)</th><th>Time</th></tr></thead>
      <tbody id="hs-tbody"><tr><td colspan="5" class="empty">No handshakes captured</td></tr></tbody>
    </table>
    </div>
  </div>
</div>

<!-- BEACON SPAM TAB -->
<div class="panel" id="tab-beacon">
  <div class="card">
    <div class="card-title">Beacon Spam Config</div>
    <div class="form-row">
      <label>Channel</label>
      <input type="number" id="bc-ch" value="6" min="1" max="165" style="width:80px">
    </div>
    <div class="form-row">
      <label>Interval (ms)</label>
      <input type="number" id="bc-int" value="100" min="10" max="5000" style="width:100px">
    </div>
    <div class="form-row">
      <label>SSIDs (one per line)</label>
      <textarea class="ssid-textarea" id="bc-ssids" placeholder="Network1&#10;Network2&#10;FreeWiFi&#10;..."></textarea>
    </div>
    <div class="form-row">
      <button class="btn btn-sec" onclick="addRandomSSIDs()">+ Random SSIDs</button>
    </div>
    <div class="toolbar" style="margin-top:8px;">
      <button class="btn btn-pri" onclick="startBeacon()">&#9889; Start Beacon Spam</button>
      <button class="btn btn-dng" onclick="stopBeacon()">&#9724; Stop</button>
    </div>
    <div id="beacon-stat" style="margin-top:8px;color:var(--muted);font-size:13px;"></div>
  </div>
</div>

<!-- LOG TAB -->
<div class="panel" id="tab-log">
  <div class="toolbar">
    <button class="btn btn-sec" onclick="fetchLog()">&#8635; Refresh</button>
    <button class="btn btn-sec" onclick="clearLog()">&#128465; Clear</button>
  </div>
  <div class="log-container" id="log-box"></div>
</div>

<!-- SETTINGS TAB -->
<div class="panel" id="tab-settings">
  <div class="card">
    <div class="card-title">Attack Settings</div>
    <div class="form-row"><label>Frames per deauth</label><input type="number" id="s-frames" value="5" min="1" max="100" style="width:80px"></div>
    <div class="form-row"><label>Send delay (ms)</label><input type="number" id="s-delay" value="5" min="0" max="1000" style="width:80px"></div>
  </div>
  <div class="card">
    <div class="card-title">Access Point</div>
    <div class="form-row"><label>SSID</label><input type="text" id="s-ssid" value="zero-bw16" maxlength="31" style="width:200px"></div>
    <div class="form-row"><label>Password</label><input type="password" id="s-pass" value="zerobw16!" maxlength="63" style="width:200px"></div>
    <div class="form-row"><label>Channel</label><input type="number" id="s-ch" value="6" min="1" max="165" style="width:80px"></div>
  </div>
  <div class="card">
    <div class="card-title">Behaviour</div>
    <div class="form-row"><label>Auto scan on boot</label><input type="checkbox" id="s-autoscan"></div>
    <div class="form-row"><label>Auto attack on boot</label><input type="checkbox" id="s-autoatk"></div>
    <div class="form-row"><label>LED enabled</label><input type="checkbox" id="s-led" checked></div>
  </div>
  <div class="toolbar">
    <button class="btn btn-pri" onclick="saveSettings()">&#10003; Save</button>
    <button class="btn btn-sec" onclick="loadSettings()">&#8635; Reload</button>
    <button class="btn btn-dng" onclick="resetSettings()">Reset Defaults</button>
  </div>
  <div class="card" style="margin-top:20px;">
    <div class="card-title">OTA Firmware Update</div>
    <div class="ota-drop" id="ota-drop" ondragover="event.preventDefault();this.classList.add('drag')" ondragleave="this.classList.remove('drag')" ondrop="otaDrop(event)">
      <div>&#128229; Drag &amp; drop firmware .bin here</div>
      <div style="font-size:12px;margin-top:6px;">or <label style="color:var(--acc);cursor:pointer;"><input type="file" id="ota-file" accept=".bin" onchange="otaFile(this)" style="display:none">click to browse</label></div>
    </div>
    <div class="ota-progress" id="ota-prog">
      <div style="margin-bottom:4px;font-size:12px;" id="ota-msg">Uploading...</div>
      <progress id="ota-bar" value="0" max="100"></progress>
    </div>
  </div>
</div>

<script>
var nets=[], selSet=new Set(), theme='dark', atkTimer=null, atkStart=0;

function showTab(t){
  document.querySelectorAll('.tab').forEach((b,i)=>b.classList.toggle('active',['attack','sniff','beacon','log','settings'][i]===t));
  document.querySelectorAll('.panel').forEach(p=>p.classList.toggle('active',p.id==='tab-'+t));
  if(t==='log') fetchLog();
  if(t==='settings') loadSettings();
}

function toggleTheme(){
  theme=theme==='dark'?'light':'dark';
  document.body.className=theme==='light'?'light':'';
  localStorage.setItem('theme',theme);
}

function setStatus(s){
  var b=document.getElementById('status-badge');
  b.className='badge '+s; b.textContent=s.toUpperCase();
}

// --- Scan ---
function doScan(){
  setStatus('scan');
  fetch('/api/scan',{method:'POST'}).then(r=>r.json()).then(d=>{
    nets=d.networks||[]; renderNetworks(); setStatus('idle');
  }).catch(e=>{setStatus('idle');});
}

function renderNetworks(){
  var tb=document.getElementById('net-tbody');
  if(!nets.length){tb.innerHTML='<tr><td colspan="10" class="empty">No networks found</td></tr>';return;}
  tb.innerHTML=nets.map((n,i)=>{
    var rssiPct=Math.max(0,Math.min(100,(n.rssi+100)*2));
    var rssiColor=rssiPct>60?'#3fb950':rssiPct>30?'#d29922':'#f85149';
    var encLabel={0:'OPEN',1:'WEP',2:'WPA',3:'WPA2',4:'WPA3',5:'WPA2/3'}[n.enc]||'?';
    var encClass={0:'enc-open',1:'enc-open',2:'enc-wpa',3:'enc-wpa2',4:'enc-wpa3',5:'enc-wpa3'}[n.enc]||'';
    var flags=(n.wps?'<span class="flag flag-wps">WPS</span>':'')+(n.pmf?'<span class="flag flag-pmf">PMF</span>':'');
    var ssidDisp=n.hidden?'<em style="color:var(--muted)">[hidden]</em>':escHtml(n.ssid);
    var checked=selSet.has(n.bssid)?'checked':'';
    return '<tr id="nr'+i+'">'
      +'<td><button class="expand-btn" onclick="toggleExpand('+i+')">&#9654;</button></td>'
      +'<td>'+i+'</td>'
      +'<td>'+ssidDisp+'</td>'
      +'<td class="mono">'+n.bssid+'</td>'
      +'<td>'+n.ch+'</td>'
      +'<td><div class="rssi-bar"><div class="rssi-fill" style="width:'+rssiPct+'%;background:'+rssiColor+'"></div></div> '+n.rssi+'</td>'
      +'<td><span class="enc-badge '+encClass+'">'+encLabel+'</span></td>'
      +'<td>'+flags+'</td>'
      +'<td>'+n.clients+'</td>'
      +'<td><input type="checkbox" '+checked+' onchange="toggleSel(\''+n.bssid+'\',this.checked)"></td>'
      +'</tr>'
      +'<tr class="sub-row" id="sub'+i+'" style="display:none"><td colspan="10"><div class="sub-inner" id="sub-inner'+i+'"></div></td></tr>';
  }).join('');
}

function toggleExpand(i){
  var row=document.getElementById('sub'+i);
  if(row.style.display==='none'){
    row.style.display=''; loadClients(i);
  } else row.style.display='none';
}

function loadClients(i){
  var net=nets[i]; if(!net) return;
  fetch('/api/sniff/clients?bssid='+net.bssid).then(r=>r.json()).then(d=>{
    var el=document.getElementById('sub-inner'+i);
    if(!d.clients||!d.clients.length){el.innerHTML='<em style="color:var(--muted)">No clients observed</em>';return;}
    el.innerHTML='<table style="margin:8px 0"><thead><tr><th>Client MAC</th><th>Last Seen</th><th>Deauth</th></tr></thead><tbody>'
      +d.clients.map(c=>'<tr><td class="mono">'+c.mac+'</td><td>'+fmtAge(c.age)+'</td>'
        +'<td><button class="btn btn-sec" onclick="deauthClient(\''+net.bssid+'\',\''+c.mac+'\')">Deauth</button></td></tr>').join('')
      +'</tbody></table>';
  });
}

function deauthClient(bssid,client){
  fetch('/api/attack/start',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify({mode:1,targets:[{bssid:bssid,client:client}],frames:5,delay:5,reason:2})});
}

function toggleSel(bssid,checked){if(checked)selSet.add(bssid);else selSet.delete(bssid);}
function selAll(v){document.querySelectorAll('#net-tbody input[type=checkbox]').forEach(c=>{c.checked=v;var bssid=c.closest('tr').querySelector('.mono').textContent;v?selSet.add(bssid):selSet.delete(bssid);});}

// --- Attack ---
function launchAttack(){
  if(!selSet.size){alert('Select at least one network');return;}
  var mode=parseInt(document.getElementById('atk-mode').value);
  var reason=parseInt(document.getElementById('atk-reason').value);
  var frames=parseInt(document.getElementById('s-frames').value)||5;
  var delay=parseInt(document.getElementById('s-delay').value)||5;
  var targets=nets.filter(n=>selSet.has(n.bssid)).map(n=>({bssid:n.bssid,client:'00:00:00:00:00:00'}));
  fetch('/api/attack/start',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify({mode,targets,frames,delay,reason})}).then(r=>r.json()).then(d=>{
    setStatus('atk');
    document.getElementById('atk-stats').style.display='flex';
    atkStart=Date.now();
    if(atkTimer) clearInterval(atkTimer);
    atkTimer=setInterval(fetchStats,1000);
  });
}

function stopAttack(){
  fetch('/api/attack/stop',{method:'POST'}).then(()=>{
    setStatus('idle');
    if(atkTimer){clearInterval(atkTimer);atkTimer=null;}
  });
}

function fetchStats(){
  fetch('/api/attack/stats').then(r=>r.json()).then(d=>{
    document.getElementById('s-deauth').textContent=d.deauths||0;
    document.getElementById('s-beacon').textContent=d.beacons||0;
    document.getElementById('s-auth').textContent=d.auth||0;
    document.getElementById('s-assoc').textContent=d.assoc||0;
    document.getElementById('pkt-count').textContent=(d.deauths||0)+(d.beacons||0)+(d.auth||0)+(d.assoc||0);
    var elapsed=Math.floor((Date.now()-atkStart)/1000);
    document.getElementById('s-elapsed').textContent=elapsed+'s';
  });
}

// --- Sniff ---
function startSniff(){
  fetch('/api/sniff/start',{method:'POST'}).then(()=>setStatus('sniff'));
  sniffPoll=setInterval(fetchSniffData,2000);
}
function stopSniff(){
  fetch('/api/sniff/stop',{method:'POST'}).then(()=>setStatus('idle'));
  if(sniffPoll){clearInterval(sniffPoll);sniffPoll=null;}
}
var sniffPoll=null;

function fetchSniffData(){
  fetch('/api/sniff/probes').then(r=>r.json()).then(d=>renderProbes(d.probes||[]));
  fetch('/api/sniff/clients').then(r=>r.json()).then(d=>renderClients(d.clients||[]));
  fetch('/api/sniff/pmkids').then(r=>r.json()).then(d=>renderPMKIDs(d.pmkids||[]));
  fetch('/api/sniff/handshakes').then(r=>r.json()).then(d=>renderHS(d.handshakes||[]));
}

function clearProbes(){fetch('/api/sniff/probes/clear',{method:'POST'}).then(fetchSniffData);}
function clearClients(){fetch('/api/sniff/clients/clear',{method:'POST'}).then(fetchSniffData);}

function renderProbes(ps){
  document.getElementById('probe-cnt').textContent=ps.length;
  var tb=document.getElementById('probe-tbody');
  if(!ps.length){tb.innerHTML='<tr><td colspan="5" class="empty">No probe requests captured</td></tr>';return;}
  tb.innerHTML=ps.map(p=>'<tr><td class="mono">'+p.mac+'</td><td>'+escHtml(p.ssid||'[broadcast]')+'</td>'
    +'<td>'+p.rssi+'</td><td>'+fmtAge(p.age_first)+'</td><td>'+p.count+'</td></tr>').join('');
}

function renderClients(cs){
  document.getElementById('client-cnt').textContent=cs.length;
  var tb=document.getElementById('client-tbody');
  if(!cs.length){tb.innerHTML='<tr><td colspan="4" class="empty">No clients observed</td></tr>';return;}
  var byAP={};
  cs.forEach(c=>{if(!byAP[c.ap])byAP[c.ap]=[];byAP[c.ap].push(c);});
  tb.innerHTML=Object.keys(byAP).map(ap=>{
    var ssid=nets.find(n=>n.bssid===ap);
    return byAP[ap].map(c=>'<tr><td class="mono">'+c.mac+'</td><td class="mono">'+c.ap+'</td>'
      +'<td>'+(ssid?escHtml(ssid.ssid):'?')+'</td><td>'+fmtAge(c.age)+'</td></tr>').join('');
  }).join('');
}

function renderPMKIDs(ps){
  document.getElementById('pmkid-cnt').textContent=ps.length;
  var tb=document.getElementById('pmkid-tbody');
  if(!ps.length){tb.innerHTML='<tr><td colspan="5" class="empty">No PMKIDs captured</td></tr>';return;}
  tb.innerHTML=ps.map(p=>'<tr><td class="mono">'+p.bssid+'</td><td>'+escHtml(p.ssid)+'</td>'
    +'<td class="mono">'+p.client+'</td><td class="mono" style="font-size:11px">'+p.pmkid+'</td>'
    +'<td>'+fmtAge(p.age)+'</td></tr>').join('');
}

function renderHS(hs){
  document.getElementById('hs-cnt').textContent=hs.length;
  var tb=document.getElementById('hs-tbody');
  if(!hs.length){tb.innerHTML='<tr><td colspan="5" class="empty">No handshakes captured</td></tr>';return;}
  tb.innerHTML=hs.map(h=>'<tr><td class="mono">'+h.bssid+'</td><td class="mono">'+h.client+'</td>'
    +'<td><span style="color:var(--grn)">'+h.msgs+'</span>/4</td>'
    +'<td class="mono" style="font-size:11px">'+h.mic+'</td><td>'+fmtAge(h.age)+'</td></tr>').join('');
}

// --- Beacon Spam ---
function addRandomSSIDs(){
  var rands=['FreeWifi','Starbucks','McDonalds','xfinitywifi','SpectrumSetup','DIRECT-TV','ATT-WiFi','Verizon','Linksys','NETGEAR'];
  var ta=document.getElementById('bc-ssids');
  ta.value+=(ta.value?'\n':'')+rands.join('\n');
}

function startBeacon(){
  var ssids=document.getElementById('bc-ssids').value.split('\n').filter(s=>s.trim());
  if(!ssids.length){alert('Enter at least one SSID');return;}
  var ch=parseInt(document.getElementById('bc-ch').value)||6;
  var intv=parseInt(document.getElementById('bc-int').value)||100;
  fetch('/api/beacon/start',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify({ssids,channel:ch,interval:intv})}).then(()=>{
    setStatus('atk');
    document.getElementById('beacon-stat').textContent='Spamming '+ssids.length+' SSIDs on ch'+ch;
  });
}

function stopBeacon(){
  fetch('/api/beacon/stop',{method:'POST'}).then(()=>{
    setStatus('idle');
    document.getElementById('beacon-stat').textContent='';
  });
}

// --- Log ---
function fetchLog(){
  fetch('/api/log').then(r=>r.json()).then(d=>{
    var box=document.getElementById('log-box');
    box.innerHTML=(d.entries||[]).map(e=>'<div class="log-'+e.level+'"><span style="color:var(--muted)">['+fmtMs(e.ts)+']</span> '+escHtml(e.msg)+'</div>').join('');
    box.scrollTop=box.scrollHeight;
  });
}

function clearLog(){
  fetch('/api/log/clear',{method:'POST'}).then(fetchLog);
}

// --- Settings ---
function loadSettings(){
  fetch('/api/settings').then(r=>r.json()).then(d=>{
    document.getElementById('s-frames').value=d.frames||5;
    document.getElementById('s-delay').value=d.delay||5;
    document.getElementById('s-ssid').value=d.ap_ssid||'zero-bw16';
    document.getElementById('s-pass').value=d.ap_pass||'';
    document.getElementById('s-ch').value=d.ap_ch||6;
    document.getElementById('s-autoscan').checked=!!d.auto_scan;
    document.getElementById('s-autoatk').checked=!!d.auto_atk;
    document.getElementById('s-led').checked=!!d.led;
  });
}

function saveSettings(){
  var cfg={
    frames:parseInt(document.getElementById('s-frames').value),
    delay:parseInt(document.getElementById('s-delay').value),
    ap_ssid:document.getElementById('s-ssid').value,
    ap_pass:document.getElementById('s-pass').value,
    ap_ch:parseInt(document.getElementById('s-ch').value),
    auto_scan:document.getElementById('s-autoscan').checked?1:0,
    auto_atk:document.getElementById('s-autoatk').checked?1:0,
    led:document.getElementById('s-led').checked?1:0,
  };
  fetch('/api/settings',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify(cfg)}).then(r=>r.json()).then(d=>{
    alert(d.ok?'Settings saved!':'Error saving');
  });
}

function resetSettings(){
  if(!confirm('Reset all settings to defaults?')) return;
  fetch('/api/settings/reset',{method:'POST'}).then(()=>loadSettings());
}

// --- OTA ---
function otaDrop(e){
  e.preventDefault();
  document.getElementById('ota-drop').classList.remove('drag');
  var f=e.dataTransfer.files[0]; if(f) doOtaUpload(f);
}
function otaFile(input){if(input.files[0]) doOtaUpload(input.files[0]);}

function doOtaUpload(file){
  if(!file.name.endsWith('.bin')){alert('Select a .bin firmware file');return;}
  if(!confirm('Flash '+file.name+' ('+Math.round(file.size/1024)+'KB)? Device will reboot.')) return;
  var prog=document.getElementById('ota-prog');
  var bar=document.getElementById('ota-bar');
  var msg=document.getElementById('ota-msg');
  prog.style.display='block'; bar.value=0;
  msg.textContent='Uploading '+file.name+'...';
  var xhr=new XMLHttpRequest();
  xhr.upload.onprogress=function(e){if(e.lengthComputable) bar.value=Math.round(e.loaded/e.total*100);};
  xhr.onload=function(){msg.textContent=xhr.status===200?'Done! Rebooting...':'Upload failed ('+xhr.status+')';};
  xhr.onerror=function(){msg.textContent='Upload error';};
  xhr.open('POST','/api/ota'); xhr.send(file);
}

// --- SSE ---
function initSSE(){
  var es=new EventSource('/events');
  es.onmessage=function(e){
    try{var d=JSON.parse(e.data);handleSSE(d);}catch(ex){}
  };
  es.onerror=function(){setTimeout(initSSE,5000);es.close();};
}

function handleSSE(d){
  if(d.type==='stats'){
    document.getElementById('pkt-count').textContent=(d.deauths||0)+(d.beacons||0)+(d.auth||0);
    if(d.deauths!==undefined) document.getElementById('s-deauth').textContent=d.deauths;
    if(d.beacons!==undefined) document.getElementById('s-beacon').textContent=d.beacons;
    if(d.status) setStatus(d.status);
  } else if(d.type==='log'){
    var box=document.getElementById('log-box');
    if(document.getElementById('tab-log').classList.contains('active')){
      var el=document.createElement('div');
      el.className='log-'+d.level;
      el.innerHTML='<span style="color:var(--muted)">['+fmtMs(d.ts)+']</span> '+escHtml(d.msg);
      box.appendChild(el); box.scrollTop=box.scrollHeight;
    }
  }
}

// --- Helpers ---
function escHtml(s){return String(s).replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;');}
function fmtMs(ms){var s=Math.floor(ms/1000);return Math.floor(s/60)+'m'+String(s%60).padStart(2,'0')+'s';}
function fmtAge(ms){if(!ms&&ms!==0)return '?';var s=Math.floor(ms/1000);if(s<60)return s+'s ago';return Math.floor(s/60)+'m ago';}

// --- Init ---
window.onload=function(){
  theme=localStorage.getItem('theme')||'dark';
  if(theme==='light') document.body.className='light';
  loadSettings();
  initSSE();
};
</script>
</body>
</html>
)rawhtml";
