if(process.env.NODE_ENV== 'production'){
    mtp = require('../build/Release/luck-node-mtp');
}
else{
    mtp = require('../build/Debug/luck-node-mtp');
}

console.log('env',process.env.NODE_ENV);
module.exports = mtp;

