const mtp = require("./binding.js");
// connect first device be found
mtp.connect();

try{
    // download from device to local
    mtp.download("data/com.ahyungui.android/db/upload.zip","/Users/tmp/upload.zip",(send,total)=>{
        console.log("progress",send,total);
    });
}
catch(e)
{
    console.error(e);
}
finally{
    // release device
    mtp.release();
}
