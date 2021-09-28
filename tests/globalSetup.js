const Zemu = require('@zondax/zemu').default;

module.exports = async () => {
    await Zemu.checkAndPullImage();
    await Zemu.stopAllEmuContainers();
};
