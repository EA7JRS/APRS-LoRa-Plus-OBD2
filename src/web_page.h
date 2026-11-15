/* Copyright (C) 2026 Manuel - EA7JRS
 *
 * This file is part of APRS LoRa+ OBD-II. GPL-3.0-or-later (ver LICENSE).
 */
// WebUI de la pasarela OBD-II → NMEA 2000. Cabecera del ecosistema APRS LoRa+ (logo, degradado verde → cian) y botón de
// donativo PayPal.me, como en el resto de equipos.
#pragma once
#include <Arduino.h>

static const char WEB_PAGE[] PROGMEM = R"HTML(<!DOCTYPE html>
<html lang="es"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>APRS LoRa+ OBD-II</title>
<link rel="icon" href="data:image/png;base64,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">
<style>
:root{--bg:#10151c;--card:#19212b;--fg:#e6edf3;--mut:#8b98a5;--acc:#4aa3ff;--red:#ff5d5d;--ok:#3fb950}
*{box-sizing:border-box}body{margin:0;font-family:system-ui,sans-serif;background:var(--bg);color:var(--fg)}
header{background:#131a22;border-bottom:2px solid transparent;border-image:linear-gradient(90deg,#3ce68a,#28b5e1) 1;height:56px;display:flex;align-items:center;gap:10px;padding:0 16px;position:sticky;top:0;z-index:10}
header .logo{height:40px;width:40px;flex-shrink:0;border-radius:8px}
header .brand{font-weight:600;color:#fff;font-size:1.1rem;display:flex;flex-direction:column;line-height:1.1}
header .brand-sub{font-weight:400;font-size:.68rem;opacity:.95;letter-spacing:.02em;color:#28b5e1}
header .right{margin-left:auto;color:#fff;opacity:.85;font-size:12px;text-align:right}
main{max-width:1100px;margin:auto;padding:12px;display:grid;gap:12px;grid-template-columns:repeat(auto-fit,minmax(320px,1fr))}
.c{background:var(--card);border-radius:10px;padding:14px}.w{grid-column:1/-1}h3{margin:0 0 10px;font-size:15px;color:var(--mut);font-weight:600}
table{width:100%;border-collapse:collapse;font-size:13px}td,th{padding:4px 6px;text-align:left;border-bottom:1px solid #26303b}
.kv td:first-child{color:var(--mut);width:52%}.okc{color:var(--ok)}.bad{color:var(--red)}.mut{color:var(--mut);font-size:13px}
button{background:#263241;color:var(--fg);border:1px solid #33475c;border-radius:8px;padding:9px 14px;font-size:14px;cursor:pointer}
button:active{background:var(--acc)}input,select{background:#0d1218;color:var(--fg);border:1px solid #33475c;border-radius:6px;padding:7px;font-size:14px}
.row{display:flex;gap:8px;flex-wrap:wrap;align-items:center;margin:6px 0}
label{font-size:13px;color:var(--mut)}label input,label select{display:block;margin-top:3px;width:100%}.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(150px,1fr));gap:8px}
a{color:var(--acc)}#msg{font-size:13px;margin-left:8px}
.btn-donate{display:inline-flex;align-items:center;gap:8px;background:#0070e0;color:#fff!important;border-radius:999px;padding:8px 18px;font-weight:600;text-decoration:none;font-size:14px}.btn-donate:hover{background:#003087;color:#fff}#about p{margin:6px 0;font-size:13px}
</style></head><body>
<header><img class="logo" src="data:image/png;base64,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" alt="APRS LoRa+" width="40" height="40"><span class="brand"><span class="brand-name">APRS LoRa+</span><small class="brand-sub">OBD-II &#8594; NMEA 2000 <span id="ver"></span></small></span><span class="right" id="hdr"></span></header>
<main>
<div class="c"><h3>Motor (OBD-II)</h3><table class="kv" id="eng"></table></div>
<div class="c"><h3>Bus OBD-II</h3><table class="kv" id="obd"></table></div>
<div class="c"><h3>NMEA 2000</h3><table class="kv" id="nk"></table></div>
<div class="c w"><h3>Configuración</h3>
<div class="grid">
<label>Nombre / punto de acceso<input id="f_name"></label>
<label>Clave de la WebUI<input id="f_pin" placeholder="(sin cambiar)"></label>
<label>WiFi<select id="f_wmode"><option value="2">Punto de acceso</option><option value="1">Cliente (+ rescate)</option><option value="0">Apagada</option></select></label>
<label>Red (SSID)<input id="f_ssid"></label>
<label>Clave de la red<input id="f_wpass" placeholder="(sin cambiar)"></label>
<label>Clave del punto de acceso<input id="f_appass" placeholder="(sin cambiar)"></label>
<label>MCP2515 CS (GPIO)<input id="f_cs" type="number"></label>
<label>Cristal MCP2515<select id="f_mhz"><option value="16">16 MHz</option><option value="8">8 MHz</option></select></label>
<label>Consulta OBD-II (ms)<input id="f_poll" type="number"></label>
<label>Caducidad del dato (ms)<input id="f_to" type="number"></label>
<label>NMEA 2000 TX (GPIO)<input id="f_tx" type="number"></label>
<label>NMEA 2000 RX (GPIO)<input id="f_rx" type="number"></label>
<label>Dirección NMEA 2000<input id="f_addr" type="number"></label>
<label>Instancia del motor<input id="f_inst" type="number"></label>
<label>MAP como sobrealimentación<select id="f_boost"><option value="1">Sí</option><option value="0">No</option></select></label>
<label>Capacidad del depósito (L)<input id="f_fuel" type="number" step="0.1"></label>
</div>
<div class="row"><button onclick="save()">Guardar</button><button onclick="post('/api/reboot')">Reiniciar</button><button onclick="if(confirm('¿Borrar la configuración?'))post('/api/reset')">Valores de fábrica</button><span id="msg"></span></div></div>
<div class="c w" id="about"><h3>Acerca de</h3>
<p><b>APRS LoRa+ OBD-II</b> &middot; &copy; 2026 Manuel - EA7JRS &middot; GPL-3.0-or-later</p>
<p>Pasarela OBD-II &#8594; NMEA 2000: los datos del motor del coche se ven como indicadores en el plotter APRS LoRa+ y en un plotter Garmin. Solo lee el coche; nunca escribe en su bus.</p>
<p><a href="https://github.com/EA7JRS/APRS-LoRa-Plus-OBD2" target="_blank" rel="noopener">github.com/EA7JRS/APRS-LoRa-Plus-OBD2</a></p>
<p>Actualizar el firmware: <a href="/update" target="_blank" rel="noopener">/update</a></p>
<p><a class="btn-donate" href="https://paypal.me/ea7jrs" target="_blank" rel="noopener">&#9829; Donar (PayPal.me/ea7jrs)</a></p></div>
</main>
<script>
const $=id=>document.getElementById(id);
function row(v){return v?'<span class="okc">sí</span>':'<span class="bad">no</span>';}
function n(v,u,d){return v===undefined||v===null?'<span class="mut">—</span>':(d!==undefined?v.toFixed(d):v)+(u||'');}
async function tick(){
 let s;try{s=await(await fetch('/api/status')).json();}catch(e){return;}
 $('ver').textContent=s.version;
 $('hdr').innerHTML=(s.wifi.ap?'AP ':'')+s.wifi.ip+(s.wifi.clients!==undefined?' · '+s.wifi.clients+' cli':'');
 const b=s.obd;
 $('eng').innerHTML=
  '<tr><td>Régimen</td><td>'+n(b.rpm,' rpm',0)+'</td></tr>'+
  '<tr><td>Velocidad</td><td>'+n(b.speed,' km/h',0)+'</td></tr>'+
  '<tr><td>Refrigerante</td><td>'+n(b.coolant,' °C',0)+'</td></tr>'+
  '<tr><td>Aceite</td><td>'+n(b.oil,' °C',0)+'</td></tr>'+
  '<tr><td>Admisión</td><td>'+n(b.intake,' °C',0)+'</td></tr>'+
  '<tr><td>Carga</td><td>'+n(b.load,' %',0)+'</td></tr>'+
  '<tr><td>Combustible</td><td>'+n(b.fuel,' %',0)+'</td></tr>'+
  '<tr><td>Consumo</td><td>'+n(b.fuelRate,' L/h',1)+'</td></tr>'+
  '<tr><td>Tensión módulo</td><td>'+n(b.volt,' V',1)+'</td></tr>'+
  '<tr><td>Presión colector</td><td>'+n(b.map,' kPa',0)+'</td></tr>'+
  '<tr><td>Marcha</td><td>'+n(b.runtime,' s',0)+'</td></tr>';
 $('obd').innerHTML=
  '<tr><td>MCP2515</td><td>'+row(b.bus)+'</td></tr>'+
  '<tr><td>ECU responde</td><td>'+row(b.ecu)+'</td></tr>'+
  '<tr><td>Consultas</td><td>'+b.requests+'</td></tr>'+
  '<tr><td>Respuestas</td><td>'+b.responses+'</td></tr>'+
  '<tr><td>Errores</td><td>'+b.errors+'</td></tr>';
 const k=s.n2k;
 $('nk').innerHTML=
  '<tr><td>Controlador</td><td>'+row(k.running)+'</td></tr>'+
  '<tr><td>Dirección</td><td>'+(k.lost?'<span class="bad">perdida</span>':k.address)+'</td></tr>'+
  '<tr><td>Reclamada</td><td>'+row(k.claimed)+'</td></tr>'+
  '<tr><td>Enviadas</td><td>'+k.sent+'</td></tr>'+
  '<tr><td>Recibidas</td><td>'+k.received+'</td></tr>'+
  '<tr><td>Cambios de dirección</td><td>'+k.claims+'</td></tr>';
}
async function load(){
 const c=await(await fetch('/api/config')).json();
 $('f_name').value=c.name;$('f_wmode').value=c.wifi.mode;$('f_ssid').value=c.wifi.ssid;
 $('f_cs').value=c.obd.cs;$('f_mhz').value=c.obd.mhz;$('f_poll').value=c.obd.pollMs;$('f_to').value=c.obd.timeoutMs;
 $('f_tx').value=c.n2k.tx;$('f_rx').value=c.n2k.rx;$('f_addr').value=c.n2k.addr;$('f_inst').value=c.n2k.instance;
 $('f_boost').value=c.n2k.mapAsBoost?'1':'0';$('f_fuel').value=c.n2k.fuelCapacityL;
}
function num(id){return parseInt($(id).value,10);}
async function save(){
 const o={name:$('f_name').value,
  wifi:{mode:num('f_wmode'),ssid:$('f_ssid').value},
  obd:{cs:num('f_cs'),mhz:num('f_mhz'),pollMs:num('f_poll'),timeoutMs:num('f_to')},
  n2k:{tx:num('f_tx'),rx:num('f_rx'),addr:num('f_addr'),instance:num('f_inst'),mapAsBoost:$('f_boost').value==='1',fuelCapacityL:parseFloat($('f_fuel').value)}};
 if($('f_pin').value)o.pin=$('f_pin').value;
 if($('f_wpass').value)o.wifi.pass=$('f_wpass').value;
 if($('f_appass').value)o.wifi.apPass=$('f_appass').value;
 const r=await fetch('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(o)});
 const j=await r.json().catch(()=>({}));
 $('msg').innerHTML=r.ok?'<span class="okc">guardado</span>':'<span class="bad">'+(j.error||'error')+'</span>';
}
async function post(u){if(u!=='/api/reboot'&&!confirm('¿Seguro?'))return;await fetch(u,{method:'POST'});$('msg').textContent='…';}
load();tick();setInterval(tick,1000);
</script></body></html>)HTML";
