const websocket = require("ws");
const inputEvent = require("input-event");

const ws = new websocket("ws://192.168.0.104/ws");

const keyboardInput = new InputEvent("/dev/input/event3"); // Change this to match your keyboard device "ls -l /dev/input/by-id/"

let duty_cycle = 200;
let left_angle = 50;
let right_angle = 130;
let duty_cycle_c = 0;
let angle_c = 90;


ws.on("open", () => {
    ws.send("js client connected");
})

ws.on("message", (stream, isBinary) => {
    if(isBinary){
        
    }
    else{
        
    }
})

ws.on("close", () => {
    console.log("web socket closed");
})

keyboardInput.on("data", event => {
    if(event.value == 1){
        console.log(`Button with code ${event.code} pressed`);
        if(event.code == 17) {
            // w
            duty_cycle_c = duty_cycle;
            sendControls(duty_cycle_c, angle_c);
        }
        else if(event.code==30){
            angle_c = left_angle;
            sendControls(duty_cycle_c, angle_c);
        }
        else if(event.code==32){
            angle_c = right_angle;
            sendControls(duty_cycle_c, angle_c);
        }
    }
    else if(event.value == 0){
        console.log(`Button with code ${event.code} released`);
        if(event.code==17){
            duty_cycle_c = 0;
            sendControls(duty_cycle_c, angle_c);
        }
    }
})

function sendControls(duty_cycle, angle){
    const controls_array = new Uint8Array([duty_cycle, angle]);
    const controls_data = Buffer.from(controls_array);
    ws.send(controls_data, {binary: true});
}