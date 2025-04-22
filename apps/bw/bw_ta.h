#ifndef TA_BITCOIN_WALLET_H
#define TA_BITCOIN_WALLET_H

/*
 * This UUID is generated with uuidgen
 * the ITU-T UUID generator at http://www.itu.int/ITU-T/asn1/uuid.html
 */
 #define TA_BITCOIN_WALLET_UUID \
 { 0x8aaaf200, 0x2450, 0x11e4, \
     { 0xab, 0xe2, 0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b} }

/* Provisioned stack size */
#define TA_STACK_SIZE			(2 * 1024)

/* Provisioned heap size for TEE_Malloc() and friends */
#define TA_DATA_SIZE			(32 * 1024)


/* Extra properties (give a version id and a string name) */
#define TA_CURRENT_TA_EXT_PROPERTIES \
    { "gp.ta.description", USER_TA_PROP_TYPE_STRING, \
        "OP-TEE Bitcoin Wallet Trusted Application" }, \
    { "gp.ta.version", USER_TA_PROP_TYPE_U32, &(const uint32_t){ 0x0010 } }

#define TA_BITCOIN_WALLET_CMD_1		1
#define TA_BITCOIN_WALLET_CMD_2		2
#define TA_BITCOIN_WALLET_CMD_3		3
#define TA_BITCOIN_WALLET_CMD_4		4
#define TA_BITCOIN_WALLET_CMD_5		5
#define TA_BITCOIN_WALLET_CMD_6		6

#define MNEMONIC_LENGTH 240
#define TA_OBJECT_MASTERKEY_EXT 2

#define TESTNET_P2PKH_PREFIX 	0x6f
#define BC_VERSION_BYTE 0x04

#endif /*TA_HELLO_WORLD_H*/