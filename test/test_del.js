const mtp = require("./binding.js");
const assert = require("assert");

function testBasic() {
    result = mtp.connect();

    assert.strictEqual(result, true);

    result = mtp.del("data/com.ahyungui.android/db/download.zip");

    assert.strictEqual(result, true);

    mtp.release();
}

assert.doesNotThrow(testBasic, undefined, "testBasic threw an expection");

console.log("Tests passed- everything looks OK!");