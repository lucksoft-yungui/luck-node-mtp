const mtp = require("./binding.js");
const assert = require("assert");

function testBasic()
{
    
    result = mtp.connect();
   
    assert.strictEqual(result,true);

    result = mtp.release();

    assert.strictEqual(result,true);

    // you can use getDeviceInfo() to get device vid and pid
    result = mtp.connect(3725,8221);
   
    assert.strictEqual(result,true);

    result = mtp.release();

    assert.strictEqual(result,true);
}

assert.doesNotThrow(testBasic, undefined, "testBasic threw an expection");

console.log("Tests passed- everything looks OK!");