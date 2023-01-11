var shell = require('shelljs');
var os = require('os');
var path = require('path')

console.log("platform", os.platform());
console.log("arch", os.arch());
console.log("version", os.version());

var _rootPath = path.dirname(__dirname);

if (os.platform() == "darwin") {
    // init share obj
    shell.exec('cp -rf /usr/local/opt/libmtp/lib/libmtp.9.dylib ' + _rootPath + '/prebuilds/darwin-x64/libmtp.dylib');
    shell.exec('cp -rf /usr/local/opt/libusb/lib/libusb-1.0.0.dylib ' + _rootPath + '/prebuilds/darwin-x64/libusb.dylib');
    shell.exec('install_name_tool -change /usr/local/opt/libmtp/lib/libmtp.9.dylib @loader_path/libmtp.dylib ' + _rootPath + '/prebuilds/darwin-x64/node.napi.node');
    shell.exec('install_name_tool -change /usr/local/opt/libusb/lib/libusb-1.0.0.dylib @loader_path/libusb.dylib ' + _rootPath + '/prebuilds/darwin-x64/libmtp.dylib');
    console.log('node.napi.node links:',shell.exec('otool -L '+_rootPath+'/prebuilds/darwin-x64/node.napi.node'));
    console.log('libmtp.dylib links:',shell.exec('otool -L '+_rootPath+'/prebuilds/darwin-x64/libmtp.dylib'));
}

