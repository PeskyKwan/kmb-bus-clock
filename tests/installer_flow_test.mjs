// Pure Node unit test with an in-memory DOM and serial device; no browser or hardware.
import assert from 'node:assert/strict';
const elements=new Map();class Element{constructor(){this.listeners={};this.value='';this.hidden=true;this.disabled=false;this.checked=false;this.textContent='';this.options=[];}addEventListener(k,v){this.listeners[k]=v;}replaceChildren(...v){this.options=v;}add(v){this.options.push(v);}scrollIntoView(){}}
globalThis.document={getElementById(id){if(!elements.has(id))elements.set(id,new Element());return elements.get(id);}};
globalThis.Option=class{constructor(text,value){this.text=text;this.value=value;}};
const fetches=[];globalThis.fetch=async path=>{fetches.push(path);return{json:async()=>({version:'0.2.0'})};};
let queue=[],resolveRead,mode='success',sent=[];const enqueue=o=>{const r={value:new TextEncoder().encode(JSON.stringify(o)+'\n'),done:false};if(resolveRead){const f=resolveRead;resolveRead=null;f(r);}else queue.push(r);};
const reader={read:()=>queue.length?Promise.resolve(queue.shift()):new Promise(r=>resolveRead=r),cancel:async()=>{if(resolveRead){resolveRead({done:true});resolveRead=null;}},releaseLock(){}};
const port={readable:null,writable:null,async open(){this.readable={getReader:()=>reader};this.writable={getWriter:()=>({releaseLock(){},write:async bytes=>{const d=JSON.parse(new TextDecoder().decode(bytes).trim());sent.push(d);if(d.cmd==='state'){enqueue({event:'state',app:'kmb-bus-clock',version:'0.2.0'});return;}const r={event:'wifi',id:d.id,stage:0,connected:true,current:'TestNet',networks:[{ssid:'TestNet',rssi:-45,open:false}],message:''};if(d.action==='connect')r.stage=4;if(d.action==='status')r.message=mode==='success'?'已連線':mode==='cancel'?'已取消':'連線失敗';if(d.action==='cancel'){mode='cancel';r.message='已取消';}enqueue(r);}})};},async setSignals(){},async close(){this.readable=null;this.writable=null;}};
Object.defineProperty(globalThis,'navigator',{value:{serial:{requestPort:async()=>port}},configurable:true});
await import('../installer/app.js');const e=id=>document.getElementById(id);
await e('existing').listeners.click();assert.equal(e('wifi-section').hidden,false);assert.equal(e('ssid').value,'TestNet');assert.equal(e('networks').options.length,2);
e('password').value='tiny';await e('connect').listeners.click();assert.match(e('wifi-status').textContent,/8–63/);
e('password').value='ExampleTest123';await e('connect').listeners.click();assert.equal(e('ready').hidden,false);assert.equal(e('password').value,'');
e('ready').hidden=true;mode='fail';e('password').value='WrongTest123';await e('connect').listeners.click();assert.equal(e('ready').hidden,true);assert.equal(e('wifi-status').textContent,'連線失敗');
mode='success';e('password').value='CancelTest123';const attempt=e('connect').listeners.click();await new Promise(r=>setTimeout(r,100));await e('cancel-network').listeners.click();await attempt;assert.equal(e('wifi-status').textContent,'已取消');
await e('disconnect').listeners.click();assert.equal(port.readable,null);assert.ok(fetches.every(p=>p==='manifest.json'));console.log('Installer setup, validation, success, rollback error, cancel, disconnect PASS; credentials only sent to mock serial');
