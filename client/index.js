const WebSocket = require("ws");
const keypress = require('keypress');

// Make `process.stdin` begin emitting "keypress" events
keypress(process.stdin);

// Configure stdin
process.stdin.setRawMode(true);
process.stdin.resume();

const ws = new WebSocket("ws://192.168.2.23/ws");

// Control states
let motor_state = 0x01;  // Default to off
let steering_state = 0xA0;  // Default to center

// Track steering direction state
let isSteeringRight = false;
let isSteeringLeft = false;

let cap_voltage = 0;

// WebSocket event handlers
ws.on("open", () => {
    console.log("Connected to WebSocket server");
    ws.send("js client connected");
});

ws.on("message", (stream, isBinary) => {
    if(isBinary) {
        const length = stream.length
        cap_voltage = stream[0];
        let current_draw = stream[1];
        console.log(`Capacitor Voltage = ${cap_voltage}`);
        console.log(`Current Draw = ${current_draw}`);
    }
    
});

ws.on("close", () => {
    console.log("web socket closed");
    process.exit();
});

// Handle keypress events
process.stdin.on('keypress', function(ch, key) {
    if (!key) return;

    // Check for Ctrl-C for exit
    if (key.ctrl && key.name === 'c') {
        console.log('Exiting...');
        process.exit();
    }

    // Handle key presses (not releases)
    if (!key.up) {
        switch(key.name) {
            case 'w':
                motor_state = 0xFF;  // Turn motor on
                console.log('Motor ON');
                break;
                
            case 's':
                motor_state = 0x01;  // Turn motor off
                console.log('Motor OFF');
                break;

            case 'd':
                if (!isSteeringRight) {
                    // Not steering right, turn right
                    steering_state = 0xA2;
                    isSteeringRight = true;
                    isSteeringLeft = false;
                    console.log('Turning RIGHT');
                } else {
                    // Already steering right, return to center
                    steering_state = 0xA0;
                    isSteeringRight = false;
                    console.log('Centering from RIGHT');
                }
                break;

            case 'a':
                if (!isSteeringLeft) {
                    // Not steering left, turn left
                    steering_state = 0xA1;
                    isSteeringLeft = true;
                    isSteeringRight = false;
                    console.log('Turning LEFT');
                } else {
                    // Already steering left, return to center
                    steering_state = 0xA0;
                    isSteeringLeft = false;
                    console.log('Centering from LEFT');
                }
                break;
        }
        sendControls();
    }
});

function sendControls() {
    const controls_data = Buffer.from([motor_state, steering_state]);
    try {
        ws.send(controls_data, { binary: true });
        console.log(`Sent: Motor=${motor_state.toString(16)}, Steering=${steering_state.toString(16)}`);
    } catch (error) {
        console.error('Error sending controls:', error);
    }
}

// Handle program exit
process.on('exit', () => {
    console.log('\nGracefully shutting down...');
    motor_state = 0x01;
    steering_state = 0xA0;
    sendControls();
    process.stdin.setRawMode(false);
    ws.close();
});

// Print initial instructions
console.log('\n=== RC Controller Started ===');
console.log('Controls:');
console.log('W: Turn motor ON');
console.log('S: Turn motor OFF');
console.log('A: Toggle LEFT turn (press again to center)');
console.log('D: Toggle RIGHT turn (press again to center)');
console.log('Ctrl+C: Quit');
console.log('========================\n');