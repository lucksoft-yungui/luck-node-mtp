const mtp = require("./binding.js");
const assert = require("assert");
var os = require('os');

function testBasic() {
    result = mtp.connect();

    assert.strictEqual(result, true);


    switch (os.platform()) {
        case "darwin":
            {
                result = mtp.upload("/Users/peiyandong/code/tmp/download.zip", "data/com.ahyungui.android/db", (send, total) => {
                    console.log("progress", send, total);
                });
            }
            break;
        case "win32": {
            result = mtp.upload("C:\\Users\\peiyandong\\Documents\\code\\temp\\download.zip", "/data/com.ahyungui.android/db", (send, total) => {
                console.log("progress", send, total);
            });
        }
    }

    assert.strictEqual(result, true);

    mtp.release();
}

assert.doesNotThrow(testBasic, undefined, "testBasic threw an expection");

console.log("Tests passed- everything looks OK!");