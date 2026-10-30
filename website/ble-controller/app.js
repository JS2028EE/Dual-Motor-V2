const SERVICE_UUID = "6e400001-b5a3-f393-e0a9-e50e24dcca9e";
const RX_UUID = "6e400002-b5a3-f393-e0a9-e50e24dcca9e";
const TX_UUID = "6e400003-b5a3-f393-e0a9-e50e24dcca9e";
const $ = selector => document.querySelector(selector);
const connectButton=$("#connectButton"), disconnectButton=$("#disconnectButton"), statusDot=$("#statusDot"), connectionStatus=$("#connectionStatus"), deviceName=$("#deviceName"), speedSlider=$("#speedSlider"), speedValue=$("#speedValue"), lastCommand=$("#lastCommand"), vehicleResponse=$("#vehicleResponse"), activityLog=$("#activityLog");
let device=null, rxCharacteristic=null, txCharacteristic=null, connected=false;
const pressedKeys = new Set();
const keyboardCommands = { w:"forward", a:"left", s:"back", d:"right" };

function log(message){const empty=activityLog.querySelector(".log-empty");if(empty)empty.remove();const row=document.createElement("p");row.className="log-entry";const time=document.createElement("span");time.className="time";time.textContent=new Date().toLocaleTimeString();row.append(time,document.createTextNode(message));activityLog.prepend(row);while(activityLog.children.length>30)activityLog.lastElementChild.remove();}
function setConnected(value){connected=value;statusDot.classList.toggle("connected",value);connectionStatus.textContent=value?"Bluetooth connected":"Not connected";connectButton.disabled=value;disconnectButton.disabled=!value;speedSlider.disabled=!value;document.querySelectorAll("[data-command]").forEach(button=>button.disabled=!value);if(!value){pressedKeys.clear();rxCharacteristic=null;txCharacteristic=null;vehicleResponse.textContent="Disconnected";}}
async function sendCommand(command){if(!connected||!rxCharacteristic){log("Not connected — command not sent.");return;}try{const bytes=new TextEncoder().encode(command);if(rxCharacteristic.writeValueWithoutResponse)await rxCharacteristic.writeValueWithoutResponse(bytes);else await rxCharacteristic.writeValue(bytes);lastCommand.textContent=command;log("TX  "+command);}catch(error){log("Send failed: "+error.message);vehicleResponse.textContent="Send failed";}}
function handleNotification(event){const value=new TextDecoder().decode(event.target.value).trim();if(!value)return;vehicleResponse.textContent=value;log("RX  "+value);}
async function connect(){if(!navigator.bluetooth){log("Web Bluetooth unavailable. Use Chrome or Edge on desktop/Android over HTTPS.");connectionStatus.textContent="Browser does not support Web Bluetooth";return;}try{connectButton.disabled=true;connectionStatus.textContent="Choose DualMotorV2…";device=await navigator.bluetooth.requestDevice({filters:[{name:"DualMotorV2"}],optionalServices:[SERVICE_UUID]});deviceName.textContent=device.name||"DualMotorV2";device.addEventListener("gattserverdisconnected",()=>{setConnected(false);log("BLE disconnected. Firmware should stop both motor outputs.");});const server=await device.gatt.connect();const service=await server.getPrimaryService(SERVICE_UUID);rxCharacteristic=await service.getCharacteristic(RX_UUID);txCharacteristic=await service.getCharacteristic(TX_UUID);await txCharacteristic.startNotifications();txCharacteristic.addEventListener("characteristicvaluechanged",handleNotification);setConnected(true);log("Connected to "+(device.name||"DualMotorV2"));vehicleResponse.textContent="Connected — ready";}catch(error){setConnected(false);connectButton.disabled=false;log(error.name==="NotFoundError"?"No device selected.":"Connection error: "+error.message);connectionStatus.textContent="Connection failed";}}
function disconnect(){if(device?.gatt?.connected)device.gatt.disconnect();setConnected(false);log("Disconnected from car.");}
connectButton.addEventListener("click",connect);disconnectButton.addEventListener("click",disconnect);
document.querySelectorAll("[data-command]").forEach(button=>button.addEventListener("click",()=>sendCommand(button.dataset.command)));

function isTypingTarget(target){return target instanceof HTMLElement && (target.isContentEditable || ["INPUT","TEXTAREA","SELECT"].includes(target.tagName));}
document.addEventListener("keydown",event=>{
  const key=event.key.toLowerCase();
  const command=keyboardCommands[key];
  if(!command || isTypingTarget(event.target)) return;
  event.preventDefault();
  if(event.repeat || pressedKeys.has(key)) return;
  pressedKeys.add(key);
  sendCommand(command);
});
document.addEventListener("keyup",event=>{
  const key=event.key.toLowerCase();
  if(!keyboardCommands[key]) return;
  pressedKeys.delete(key);
  if((key==="w" || key==="s") && !pressedKeys.has("w") && !pressedKeys.has("s")){
    sendCommand("stop");
  }
});
window.addEventListener("blur",()=>{
  const wasDriving=pressedKeys.size>0;
  pressedKeys.clear();
  if(wasDriving) sendCommand("stop");
});

speedSlider.addEventListener("input",()=>speedValue.textContent=speedSlider.value);
speedSlider.addEventListener("change",()=>sendCommand("speed_"+Number(speedSlider.value)));
$("#clearLog").addEventListener("click",()=>activityLog.innerHTML='<p class="log-empty">Activity cleared.</p>');
setConnected(false);
