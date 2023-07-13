import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import chunkBuffer, { toHex } from './helpers';
import { setupZemu } from './options';

enum CredentialDeploymentType {
    NEW,
    EXISTING,
}

/**
 * Handles (sends data and performs UI actions) the public key part
 * of the credential deployment transaction.
 */
async function handleCredentialDeploymentKeys(sim: Zemu, transport: Transport, ins: number, p2: number, numberOfKeys: number, navigateKeys?: () => Promise<void>) {
    let data = Buffer.from(toHex(numberOfKeys), 'hex');
    await transport.send(0xe0, ins, 0x0A, p2, data);
    let showIntro = true;

    // Send all keys except the final key and handle navigating the UI for the keys.
    // This is only relevant if there are multiple keys, as the UI for the final key
    // is deferred to the end of the flow.
    for (let i = 0; i < numberOfKeys - 1; i++) {
        data = Buffer.from(toHex(i + 1) + '00f78929ec8a9819f6ae2e10e79522b6b311949635fecc3d924d9d1e23f8e9e1c3', 'hex');
        transport.send(0xe0, ins, 0x01, p2, data);

        if (showIntro) {
            showIntro = false;
            await sim.clickRight(undefined, true);
        }

        if (navigateKeys) {
            await navigateKeys();
        } else {
            throw new Error('Navigate keys should be defined when sending multiple keys.');
        }
    }

    data = Buffer.from('0000f78929ec8a9819f6ae2e10e79522b6b311949635fecc3d924d9d1e23f8e9e1c3', 'hex');
    await transport.send(0xe0, ins, 0x01, p2, data);
}

/**
 * Handles the shared part of credential deployment / update credentials.
 */
async function sharedCredentialDeployment(
    sim: Zemu,
    transport: Transport,
    ins: number,
    p2: number,
    numberOfKeys?: number,
    navigateKeys?: () => Promise<void>
) {
    await handleCredentialDeploymentKeys(sim, transport, ins, p2, numberOfKeys ? numberOfKeys : 1, navigateKeys);

    let data = Buffer.from('ff85d8a7aa296c162e4e2f0d6bfbdc562db240e28942f7f3ddef6979a1133b5c719ec3581869aaf88388824b0f6755e63c0000f013010001', 'hex');
    await transport.send(0xe0, ins, 0x02, p2, data);

    data = Buffer.from('000f0301aca024ce6083d4956edad825c3721da9b61e5b3712606ba1465f7818a43849121bdb3e4d99624e9a74b9436cc8948d178b9b144122aa070372e3fadee4998e1cc21161186a3d19698ad245e10912810df1aaddda16a27f654716108e27758099', 'hex');
    await transport.send(0xe0, ins, 0x03, p2, data);

    data = Buffer.from('07e40b07e10c0001', 'hex');
    await transport.send(0xe0, ins, 0x04, p2, data);

    data = Buffer.from('010000000000000004', 'hex');
    await transport.send(0xe0, ins, 0x05, p2, data);

    data = Buffer.from('4a6f686e', 'hex');
    await transport.send(0xe0, ins, 0x06, p2, data);

    data = Buffer.from('00000831', 'hex');
    await transport.send(0xe0, ins, 0x07, p2, data);

    const proofs = Buffer.from('b8dc01d4fdd0c1b455e9a48285eac39ffc0a433929bbd29344016a2fdbd3892fabed9a607da37140ea2046aa7c924022a130cc78688e8174c4c244ea97461007b167f4c941585ff9b945eddff0f87942bf91bedc8d33355e7f3e09e4168c6ec98e3aa4b7b2e6a7807dd5cc6b057646115b82c65c97e3bc7a5898c61d99465ad655df2628d3d38cf97f7134fddfae366f80755e0dd926c54a74d05724da31c960aa5322e76a98af78e0d4ac161fd0af244a6e1068ed7661fdf0d4669c2cad41cd87641313d9ba06feb204f6c26ed208a7479daa933450a1fc293e792d6387724748a4d9403e7ff1f1deacbe41c949b06c000101a24eeaf8709e6e9930980d0a9ad6585760e8d9b58c04c6c73ce2245b70e237c86a61289dbb1cb59ff23e406a3426a92900000000000000019237d759d6dc3402310bc58ae027cbe65e4818f26fc2480a023d99a9af5403e8db6c834d6b964d42c9703959ed0121089a36989c8b92d83af2547472eeda05246837f7b3691732291a39ae7497147047000000020000000131f676f011cc5bde75b16440b051d0020070137bec8be3f167a961fbf99f647070298bc8c7f572bb3d5c8dc3b5d4c9f9750d54c85f8388e16e4bf0644c48c5666b02fec6f186c4b36994396f7023ef23c14535129c738664392d96e486eb15130000000259f75638807a180c00db02ef4dd3639132edbafc3663677494e8b1e2c51567a82e353e70c5e4c06213b09e5c528d8695cb5fd8ba97593c7ae9822f1835de801f18fce82ebf4839ca5062289eeb8ff36ded218404ccf7248008d1e72683b1138e4ab37e620b34b5793a2109e0734064ad03c5c5c14fb9920bf1f8c397b849b223000000085e17de448c17d47a4f715601fc66c5ddaea6154cd909795da984a099105af7486b2c1d617335d7a9d0d3db0c2c6a460e3680ff7b3d9d4bdc851494e90ff5101a1dc7323925c2e3f1c829199364456fc3dde9d770498b12a32d6b570c1c7221d859fe628d7e5a31711239e4d5eccc5bb7204f6e09cbfe00373e1042ac79dfbb7d4bf280fd53e8a0bac84bc6928c874ebf18905d50396083c7dd28455b68f37cf36f7b85b1027be4e1d2ccf8d2193e3436234bb4377724b0900db3e0f5c6466e2d083e97b933e49a9e0290b46b3acf81a01792954a1e1725793fb5ef4b596bd9a31c48285e3d056e79c6cd0094f8462b9e510c17d75d02639fbab819afbf1c84a55618932d52113990ef19268a2ebc0778b1b567e31a94264d6ac9cfc03867c24d50cfbf02f74dc8c9de3ac039e61ccd1865aa75d2a6f7627291fdae75c474ed3943d56c5a926eb6e6c121c550bcd4de0a0739021c50672407def5836c60d24ec762c839b6dee6cf937412ae6757d6894c8ff8dec2c0d6f3cd993e808965c84eb413d6022be0b1b1ccac25796adc0d95a2d7775e87a4d8085fa8a43f71008df77911b6f5b4ce5ee5d6f5412966ca47b20af751752fc0879fcc8a0d879acdbc8bcb00dbb93f0e10bcf36ccaf001191a4e4f47d40f59451fe4b376f2dd1d4d58854a55ed74ff2c5bb59c531e5651c99afb04318d84d936bc7ebba3823e108d7ae246157c7851016eb4bd9e74cd15af484be18b11115a2ef091e2439fce06426d4ff063dc86ffe0c5a3ea5f920fe3e8c946a5d0eacf4439f4beca8d8dc6311343f516444ef6779e3170a14b98ca26bbe516397a4f17b395178f473376a6944d0d1aba4fea8c66abaf63aef20f364c73f2c84cb3ef66f8ded751e7c8229b87f7276ba8474cced9521caafa0836e9cbb64aeaa86f20a7a38cb5353af7d642949fd59ec40100aa70e4ddbf823a5e89cb39328f7fcb2dd07b15652c3a027d5a8fc982abf950f355455937a5810cc0bb7e03b11db4bc8988b06c1467fe716a43b2ca13aa56f20bb651451fb838a93e8345a1b0f782edc37ad6b5d2719d2893e4585938f663d9df10a0bc8434b6dafd883b4c2b64b02a19b0c0d72ca2e5588e5df29963a423592917d86c372532fc65e1d2d85913c32e88466b01d5f5539079b1e2521858db1a9e9317c3ecdbed418a7caefa3da589436f619b2a617c078bf07ad753c27fb72b3731e459764264df77b1af2ca63644c2938055b0d73ceb1d8a2a50592ce06edf254709dd26f277f0af119c2e85b175070554b5e48101bdaed6df86e599f2673fae59d69eb3876c2ca7b96ae8f26428e13b45daa6b7fa5766adc5a45af6c300797e61a7c00cc186b3a0adb7ffa7f58eca72cb889c72347c1673846bdd28af281d2129934cc7347e92904422367bbc26000f70621f866f6fd5f1e9e14c65027a1179000000048415e12f31d099a3becd3c0a3304704980718faa6a2b73cad42ddbe0f0fd388c30bc668f3c70f52215bdf493ea6704b9864f06b92ccc8488090092ae5bc248ea3e20f77f59985ed062f46e8e3a2e1119fb97021fb20c432cfc8dd29a70a26b9bb80fa4e6b9c14e0299088f675e25a9f8c9192fb9bd8a672e71fb02a0275e9090f4e5fd3ea9d91daf731efcc696b24222b13931e1a82b858cd636ea690900c8e0608132bca8ec9bfa522112f8b9b3c6e790c68f9d0fd78750148dbc698e36d3f1a829a08d1dda9e66e552e282fade22759736b4677a1c23f5af0956923df8d0675adbaf6b2b7ee4e1ab4ffcb53828dcefb0a432370101feaccd66ef1cee80e2253f3a0d8024ff09d6ceef4742424a3a9f844e6854b32a5d4b66a71b3cc5c32e9cb9fe53a5959a4108090385951361357a61185398d361bab4804479a7712b07161a9b74e7760168cc70f5bbacc817ad208a7bc867a3e01f46e6832fb08ea77a4784b9e93b352a5a0d55646b099dfe7bd619625406828dc0a3d19da86c7550e44301c0decaab090bc3e146559388bcde1b4edc0fb76e152c937c4edb3c60179a8668a8baf73acda699950bf98d40dffc6d792a4702583dbc0afc34b6bb462e70f3', 'hex');
    const chunkedProofs = chunkBuffer(proofs, 255);

    for (const chunk of chunkedProofs) {
        await transport.send(0xe0, ins, 0x08, p2, chunk);
    };
}

async function credentialDeployment(
    sim: Zemu, transport: Transport, type: CredentialDeploymentType, expectedSignature: string,
    handleKeyUi: () => Promise<any>,
    handleAddressUi: () => Promise<void>,
    numberOfKeys?: number,
    navigateKeys?: () => Promise<void>
) {
    let data = Buffer.from('080000045100000000000000000000000000000000000000020000000000000000', 'hex');
    await transport.send(0xe0, 0x04, 0x00, 0x00, data);

    await sharedCredentialDeployment(
        sim, transport, 0x04, 0x00, numberOfKeys, navigateKeys
    );

    let tx;
    if (type === CredentialDeploymentType.NEW) {
        data = Buffer.from('00000000006040F27E', 'hex');
        tx = transport.send(0xe0, 0x04, 0x09, 0x00, data);
        await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
        await handleKeyUi();
    } else if (type === CredentialDeploymentType.EXISTING) {
        data = Buffer.from('0120a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7', 'hex');
        tx = transport.send(0xe0, 0x04, 0x09, 0x00, data);
        await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
        await handleAddressUi();
    }

    await expect(tx).resolves.toEqual(
        Buffer.from(expectedSignature, 'hex'),
    );
}

async function updateCredentials(
    sim: Zemu, transport: Transport,
    handleInitialUi: () => Promise<any>,
    handleCredentialUi: () => Promise<any>,
) {
    const data = Buffer.from('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71402', 'hex');
    transport.send(0xe0, 0x31, 0x00, 0x00, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await handleInitialUi();

    for (let i = 0; i < 2; i += 1) {
        const credentialIndex = Buffer.from(`0${i}`, 'hex');
        await transport.send(0xe0, 0x31, 0x00, 0x01, credentialIndex);
        await sharedCredentialDeployment(
            sim, transport, 0x31, 0x02
        );
    }

    const credentialId1 = Buffer.from('85d8a7aa296c162e4e2f0d6bfbdc562db240e28942f7f3ddef6979a1133b5c719ec3581869aaf88388824b0f6755e63c', 'hex');
    const credentialId2 = Buffer.from('aca024ce6083d4956edad825c3721da9b61e5b3712606ba1465f7818a43849121bdb3e4d99624e9a74b9436cc8948d18', 'hex');
    const credentialIdList = [credentialId1, credentialId2];

    await transport.send(0xe0, 0x31, 0x00, 0x03, Buffer.from('02', 'hex'));

    let credentialSnapshot = sim.getMainMenuSnapshot();
    for (const credentialId of credentialIdList) {
        transport.send(0xe0, 0x31, 0x00, 0x04, credentialId);
        await sim.waitUntilScreenIsNot(credentialSnapshot);
        credentialSnapshot = await handleCredentialUi();
    }

    const threshold = Buffer.from('02', 'hex');
    const tx = transport.send(0xe0, 0x31, 0x00, 0x05, threshold);
    await sim.waitUntilScreenIsNot(credentialSnapshot);
    await sim.clickRight();
    await sim.clickBoth(undefined, false);

    await expect(tx).resolves.toEqual(
        Buffer.from('f51a30bc002751498b348f5b7ad8bd73afd20126aa498f8eede9840bc6d2f9e72342297528404baf3a89b4b67cba04dab4b3fd75248ffb42037bb546bf67a7019000', 'hex'),
    );
}

test('[NANO S] Update credentials', setupZemu('nanos', async (sim, transport) => {
    await updateCredentials(
        sim,
        transport,
        async () => {
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight(undefined, false);
            return sim.clickBoth(undefined, false);
        },
        async () => {
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight(undefined, false);
            return sim.clickBoth(undefined, false);
        },
    );
}));

async function updateCredentialsXAndSP(sim: Zemu, transport: Transport) {
    await updateCredentials(
        sim,
        transport,
        async () => {
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight(undefined, false);
            return sim.clickBoth(undefined, false);
        },
        async () => {
            await sim.clickRight();
            await sim.clickRight(undefined, false);
            return sim.clickBoth(undefined, false);
        },
    );
}

test('[NANO SP] Update credentials', setupZemu('nanosp', updateCredentialsXAndSP));

test('[NANO X] Update credentials', setupZemu('nanox', updateCredentialsXAndSP));

test('[NANO S] Credential deployment for new account with multiple keys', setupZemu('nanos', async (sim, transport) => {
    await credentialDeployment(
        sim,
        transport,
        CredentialDeploymentType.NEW,
        'd68da82666d2e7eddbef3cb5cbeb95bfec62ee105e5f5a63d0db5fdc51934cfc6c0509316c06b384fa7df08eecafee9bbff378bc90ff436f26fa2ac8159ecd049000',
        async () => {
            await sim.navigate('.', 'multiple keys', [6, 0], true, false);
        },
        async () => {
            // Not used for new account
        },
        3,
        async () => {
            await sim.navigate(".", 'multiple-keys', [4], true, false)
            await sim.clickBoth(undefined, false);
        },
    );
}));


test('[NANO S] Credential deployment for new account', setupZemu('nanos', async (sim, transport) => {
    await credentialDeployment(
        sim,
        transport,
        CredentialDeploymentType.NEW,
        '3a7d502e14ffd0e833bde629c798afc0721a6964af71f1c4dc54fdcde6ba0ce13db70f58bf9b3c2b9bd3fa100e1d6e21fe8b902237264c8662307bc8f7634f019000',
        async () => {
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight();
            return sim.clickBoth(undefined, false);
        },
        async () => {
            // Not used for new account
        },
    );
}));

test('[NANO S] Credential deployment for an existing account', setupZemu('nanos', async (sim, transport) => {
    await credentialDeployment(
        sim,
        transport,
        CredentialDeploymentType.EXISTING,
        '3ccfcb7408e20a322dcf5bad01e285ded3f97df4ceb31851597222a4bdbd9a471c70d08bf51b583ee8552fa0b6bae00565a2ff36ba1bfa44a52a4a73f370b2079000',
        async () => {
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight();
            return sim.clickBoth(undefined, false);
        },
        async () => {
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickBoth(undefined, false);
        },
    );
}));

async function credentialDeploymentNewAccountXAndSP(sim: Zemu, transport: Transport) {
    await credentialDeployment(
        sim,
        transport,
        CredentialDeploymentType.NEW,
        '3a7d502e14ffd0e833bde629c798afc0721a6964af71f1c4dc54fdcde6ba0ce13db70f58bf9b3c2b9bd3fa100e1d6e21fe8b902237264c8662307bc8f7634f019000',
        async () => {
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight();
            return sim.clickBoth(undefined, false);
        },
        async () => {
            // Not used for new account
        },
    );
}

test('[NANO SP] Credential deployment for new account', setupZemu('nanosp', credentialDeploymentNewAccountXAndSP));

test('[NANO X] Credential deployment for new account', setupZemu('nanox', credentialDeploymentNewAccountXAndSP));

async function credentialDeploymentExistingAccountXAndSP(sim: Zemu, transport: Transport) {
    await credentialDeployment(
        sim,
        transport,
        CredentialDeploymentType.EXISTING,
        '3ccfcb7408e20a322dcf5bad01e285ded3f97df4ceb31851597222a4bdbd9a471c70d08bf51b583ee8552fa0b6bae00565a2ff36ba1bfa44a52a4a73f370b2079000',
        async () => {
            await sim.clickRight(undefined, false);
            return sim.clickBoth(undefined, false);
        },
        async () => {
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight(undefined, false);
            await sim.clickBoth(undefined, false);
        },
    );
}

test('[NANO SP] Credential deployment for an existing account', setupZemu('nanosp', credentialDeploymentExistingAccountXAndSP));

test('[NANO X] Credential deployment for an existing account', setupZemu('nanox', credentialDeploymentExistingAccountXAndSP));

async function credentialDeploymentNewAccountMultipleKeysXAndSP(sim: Zemu, transport: Transport) {
    await credentialDeployment(
        sim,
        transport,
        CredentialDeploymentType.NEW,
        'd68da82666d2e7eddbef3cb5cbeb95bfec62ee105e5f5a63d0db5fdc51934cfc6c0509316c06b384fa7df08eecafee9bbff378bc90ff436f26fa2ac8159ecd049000',
        async () => {
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight();
            return sim.clickBoth();
        },
        async () => {
            // Not used for new account
        },
        3,
        async () => {
            await sim.navigate(".", 'multiple-keys', [2], true, false)
            await sim.clickBoth(undefined, false);
        }
    );
}

test('[NANO SP] Credential deployment for new account with multiple keys', setupZemu('nanosp', credentialDeploymentNewAccountMultipleKeysXAndSP));

test('[NANO X] Credential deployment for new account with multiple keys', setupZemu('nanox', credentialDeploymentNewAccountMultipleKeysXAndSP));
