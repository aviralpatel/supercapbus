const InputEvent = require('input-event');
const fs = require('fs');

// Find your keyboard device (usually something like /dev/input/event0 through event9)
// You'll need to run this as root (sudo) to access the input device
const input = new InputEvent('/dev/input/event3'); // Change this to match your keyboard device "ls -l /dev/input/by-id/"

input.on('error', error => {
    console.error('Error reading input:', error);
    process.exit(1);
});

input.on("data", (event) =>{
    if(event.value == 1){
        console.log(`key pressed with code ${event.code}`)
    }
    else if(event.value == 0){
        console.log(`key released with code ${event.code}`)
    }
})