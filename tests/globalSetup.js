const Zemu = require('@zondax/zemu').default;

const catchExit = () => {
    process.on('SIGINT', () => {
        Zemu.stopAllEmuContainers()
    })
}

module.exports = async () => {
    catchExit();
    await Zemu.checkAndPullImage();
    await Zemu.stopAllEmuContainers();
};
