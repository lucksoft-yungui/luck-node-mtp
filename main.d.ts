declare module 'luck-node-mtp' {
    interface ListObject {
      name: string,
      size: number,
      type: string,
      id: number,
      modificationdate: number,
      parent_id: number,
      storage_id: number,
    }

    interface DeviceInfo {
      vendor: string,
      vendor_id: number,
      product: string,
      product_id: number
    }

    interface StorageInfo {
      id: number,
      StorageDescription: string,
      VolumeIdentifier: string,
    }

    /**
     * Connect to a device.
     *
     * @param {number} vendorId
     * @param {number} productId
     *
     * @return {boolean}
     */
    export function connect(vendorId?: number, productId?: number): boolean;

    /**
     * Release the currently connected device.
     *
     * @return {boolean}
     */
    export function release(): boolean;

    /**
     * Get a list of objects within a given mtp device parent path. A file tree can be build by using this method.
     *
     * @param {string} parentPath
     *
     * @return {Array.<ListObject>}
     */
    export function getList(parentPath: string): [ListObject];

    /**
     * Returns a list of valid connected devices.
     *
     * @return {Array.<DeviceInfo>}
     */
    export function getDeviceInfo(): [DeviceInfo];

    /**
     * Upload a file to the MTP device.
     *
     * @param {string} sourcePath
     * @param {string} targetPath
     * @param {Function} callback
     *
     * @return {boolean}
     */
    export function upload(sourcePath: string, targetPath: string, callback?: Function): boolean;

    /**
     * This function deletes a single file, track, playlist, folder or any other object from the MTP device, identified by the object ID.
     *
     * @param {string} sourcePath
     * @param {string} targetPath
     * @param {Function} callback
     *
     * @return {boolean}
     */
    export function del(sourcePath: string, targetPath: string, callback?: Function): boolean;

    /**
     * Obtain information about an object on the device.
     *
     * @param {string} targetPath
     *
     * @return {ListObject}
     */
    export function getObject(targetPath: string): ListObject;

    /**
     * Copy a file from one place on the device to another place on the device.
     *
     * @param {string} sourcePath
     * @param {string} targetPath
     *
     * @return {boolean}
     */
    export function copy(sourcePath: string, targetPath: string): boolean;

    /**
     * Move a file from one place on the device to another place on the device.
     *
     * @param {string} sourcePath
     * @param {string} targetPath
     *
     * @return {boolean}
     */
    export function copy(sourcePath: string, targetPath: string): boolean;

    /**
     * Set the file name of an object on the device.
     * @param {string} sourcePath
     * @param {string} filename
     *
     * @return {boolean}
     */
    export function setFileName(sourcePath: string, filename: string): boolean;

    /**
     * Set the name of a folder on the device.
     *
     * @param {string} sourcePath
     * @param {string} foldername
     *
     * @return {boolean}
     */
    export function setFolderName(sourcePath: string, foldername: string): boolean;

    /**
     * Create a folder on the device.
     *
     * @return {number} The object id of the new folder.
     */
    export function createFolder(targetPath: string, foldername: string): number;

    /**
     * Get the storage information of the currently connected device.
     *
     * @return {StorageInfo}
     */
    export function getCurrentDeviceStorageInfo(): [StorageInfo];

    /**
     * Select the device storage for the currently connected device.
     *
     * @param {number} storageId
     *
     * @return {boolean}
     */
    export function setStorage(storageId: number): boolean;
  }
