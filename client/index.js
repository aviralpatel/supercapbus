const websocket = require("ws");
const inputEvent = require("input-event");

const ws = new websocket("ws://192.168.0.104/ws");

const keyboardInput = new InputEvent("/dev/input/event3"); // Change this to match your keyboard device "ls -l /dev/input/by-id/"

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

keyboardInput.on("data", eventt => {

})
