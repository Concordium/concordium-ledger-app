const Zemu = require('@zondax/zemu').default;

const catchExit = async () => {
    process.on('SIGINT', () => {
        Zemu.stopAllEmuContainers()
    })
}

module.exports = async () => {
    await catchExit()
    await Zemu.checkAndPullImage()
    await Zemu.stopAllEmuContainers()
};
