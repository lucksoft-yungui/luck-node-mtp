const mtp = require("./binding.js");
const assert = require("assert");
var os = require('os');

function testBasic()
{
    result = mtp.connect();
   
    assert.strictEqual(result,true);

    switch(os.platform()){
        case "darwin":
            {
                result =  mtp.download("data/com.ahyungui.android/db/upload.zip","/Users/peiyandong/code/tmp/upload.zip",(send,total)=>{
                    console.log("progress",send,total);
                });
            }
         break;
         case "win32":{
                result =  mtp.download("data/com.ahyungui.android/db/upload.zip","C:\\Users\\peiyandong\\Documents\\code\\temp\\upload.zip",(send,total)=>{
                    console.log("progress",send,total);
                });
         }
    }

    assert.strictEqual(result,true);

    mtp.release();
}

assert.doesNotThrow(testBasic, undefined, "testBasic threw an expection");

console.log("Tests passed- everything looks OK!");