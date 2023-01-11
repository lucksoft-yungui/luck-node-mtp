const mtp = require("./binding.js");
const assert = require("assert");


function isNumeric(value) {
    return /^\d+$/.test(value);
}

function testBasic() {
    result = mtp.connect();

    assert.strictEqual(result, true);

    result = mtp.createFolder("/data/com.ahyungui.android/db", "test_folder");

    assert.strictEqual(isNumeric(result), true);

    mtp.release();
}

assert.doesNotThrow(testBasic, undefined, "testBasic threw an expection");

console.log("Tests passed- everything looks OK!");