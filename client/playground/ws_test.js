const websocket = require("ws");


const ws = new websocket("ws://192.168.0.104/ws");

ws.on("open", () => {
    ws.send("hello from js client");
    const binary_data = Buffer.from([0xff, 0x8e, 0x77, 0x01]);
    ws.send(binary_data, {binary: true});
})

ws.on("message", (stream, isBinary) => {
    if(isBinary){
        const data_length = stream.length;
        console.log(`Binary Data Length- ${data_length}`);
        for(let i=0; i<data_length; i++){
            console.log(stream[i]);
        }
    }
    else{
        console.log(`Text Data- ${stream}`);
    }
})