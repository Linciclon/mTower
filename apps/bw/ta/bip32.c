#include <bip32.h>
#include <string.h>
#include <stdbool.h>
#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
#include <inttypes.h>
#include <secp256k1.h>
#include "bignum.h"

static void print_uint8(uint8_t* array, uint32_t array_len){
	uint32_t i;
	for(i=0; i<array_len; i++){
		printf("%x", array[i]);
	}
	printf("\n");
}


TEE_Result hdnode_from_seed(const uint8_t *seed, int seed_len, uint8_t* master_sk, uint8_t* master_chaincode)
{
	TEE_OperationHandle op = (TEE_OperationHandle)NULL;
	TEE_ObjectHandle key = (TEE_ObjectHandle)NULL;
	TEE_Attribute attr;
	TEE_Result res;

	uint8_t mac[64];
	const char key_str[512/8] = "Bitcoin seed";
	uint32_t key_len = sizeof(key_str);
	size_t mac_len = 64;
	uint8_t il[32];
	uint8_t ir[32];
	
	// mac operation must be init before compute final, and the operator has to have a key
	res = TEE_AllocateOperation(&op, TEE_ALG_HMAC_SHA512, TEE_MODE_MAC, 512);
	if(res != TEE_SUCCESS) EMSG("Failed in allocating operation: 0x%x", res);

	res = TEE_AllocateTransientObject(TEE_TYPE_HMAC_SHA512, 512, &key);
	if(res != TEE_SUCCESS) EMSG("Failed in allocating transient objec: 0x%x", res);
	
	TEE_InitRefAttribute(&attr, TEE_ATTR_SECRET_VALUE, key_str, key_len);

	res = TEE_PopulateTransientObject(key, &attr, 1);
	if(res != TEE_SUCCESS) EMSG("Failed in populating transient object: 0x%x", res);
	
	res = TEE_SetOperationKey(op,key);
	if(res != TEE_SUCCESS) EMSG("Failed in seting operation key: 0x%x", res);

	TEE_MACInit(op, NULL, 0);
	TEE_MACUpdate(op, seed, seed_len);

	res = TEE_MACComputeFinal(op, NULL, 0, mac, &mac_len);
	if(res != TEE_SUCCESS) EMSG("Failed in computing final mac: 0x%x", res);

	TEE_MemMove(il, mac, 32);
	TEE_MemMove(ir, mac+32, 32);

	TEE_MemMove(master_sk, il, 32);
	TEE_MemMove(master_chaincode, ir, 32);

	return TEE_SUCCESS;
}

/*
	Bitcoin ECDSA public keys represent a point on a particular Elliptic Curve (EC) defined in secp256k1.
	use hardened child key only, which only allows user to generate child key from parent private key
 */
TEE_Result hdnode_private_ckd(uint8_t* parent_sk, uint8_t* parent_chaincode, uint32_t i, uint8_t* child_sk, uint8_t* child_chaincode)
{
	TEE_Result res;
	TEE_Attribute attr;
	TEE_ObjectHandle key = (TEE_ObjectHandle)NULL;
	TEE_OperationHandle op = TEE_HANDLE_NULL;
	uint8_t mac[64];
	uint8_t entropy[37];
	size_t mac_len = 64;
	uint8_t il[32];
	uint8_t ir[32];
	bignum256 a;
	bignum256 b;

	const ecdsa_curve* curve = &secp256k1;

	entropy[0] = 0;
	TEE_MemMove(entropy+1, parent_sk, 32);
	entropy[33] = 0x80;
	entropy[34] = 0;
	entropy[35] = 0;
	entropy[36] = 0;


	// HMAC-SHA512 (parent_chain, 0x00||parent_sk||i)
	res = TEE_AllocateOperation(&op, TEE_ALG_HMAC_SHA512, TEE_MODE_MAC, 512);
	if(res != TEE_SUCCESS) EMSG("Failed in allocating operation: 0x%x", res);
	res = TEE_AllocateTransientObject(TEE_TYPE_HMAC_SHA512, 512, &key);
	if(res != TEE_SUCCESS) EMSG("Failed in allocating transient objec: 0x%x", res);
	 // Create the attribute with the parent chain code (parent chain code is the key)
	TEE_InitRefAttribute(&attr, TEE_ATTR_SECRET_VALUE, parent_chaincode, 32);
	 // Populate the key attributes
	res = TEE_PopulateTransientObject(key, &attr, 1);
	if(res != TEE_SUCCESS) EMSG("Failed in populating transient object: 0x%x", res);
	res = TEE_SetOperationKey(op,key);
	if(res != TEE_SUCCESS) EMSG("Failed in seting operation key: 0x%x", res);

	// Initialize the HMAC operation
	TEE_MACInit(op, NULL, 0);
	TEE_MACUpdate(op, entropy, 37);
	res = TEE_MACComputeFinal(op, NULL, 0, mac, &mac_len);
	if(res != TEE_SUCCESS) EMSG("Failed in computing final mac: 0x%x", res);

	//printf("MAC Value:");
	//print_uint8(mac, 64);

	TEE_MemMove(il, mac, 32);
	TEE_MemMove(ir, mac+32, 32);
	
	bn_read_be(parent_sk, &a);

	while(true){
		bool failed = false;
		bn_read_be(il, &b);
		if(!bn_is_less(&b, &curve->order)){// b >= order
			failed = true;
		}else{
			bn_add(&b, &a);
			bn_mod(&b, &curve->order);
			if(bn_is_zero(&b)){
				failed = true;
			}
		}
		if(!failed){
			bn_write_be(&b, child_sk);
			break;
		}
	}

	TEE_MemMove(child_chaincode, ir, 32);

	TEE_MemFill(&op, 0, sizeof(op));
	TEE_MemFill(&key, 0, sizeof(op));

	return TEE_SUCCESS;

	//big-endian oirque a posição 0 é o byte mais significativo
	// convert a raw bigendian 256 bit value (parent_sk) into a normalized bignum.
	// "a" is partly reduced (since it fits in 256 bit)
	// bn_read_be(parent_sk, &a);

	// while(true){
	// 	bool failed = false;
	// 	bn_read_be(il, &b);
	// 	if(!bn_is_less(&b, &curve->order)){// b >= order
	// 		failed = true;
	// 	}else{
	// 		// add two numbers b = b + a
	// 		// assumes that b, a are normalized
	// 		// guarantees that b is normalized
	// 		bn_add(&b, &a);
	// 		// compute b = b mod curve->order by computing  b >= curve->order ? b - curve->order : b.
	// 		// assumes b partly reduced, guarantees b fully reduced.
	// 		bn_mod(&b, &curve->order);
	// 		if(bn_is_zero(&b)){
	// 			failed = true;
	// 		}
	// 	}
	// 	if(!failed){
	// 		bn_write_be(&b, child_sk);
	// 		break;
	// 	}
	// }

	// printf("child_sk b2(512): 0x%x \n", child_sk);
	// print_uint8(child_sk, 32);

	// bn_read_be(il, &b);
	// if(!bn_is_less(&b, &curve->order)){// b >= order
	// 	EMSG("Failed in computing elliptic curve cryptographic key derivation!");
	// 	return TEE_ERROR_BAD_PARAMETERS; //choose another type of errot
	// }

	// add two numbers b = b + a
	// assumes that b, a are normalized
	// guarantees that b is normalized
	// bn_add(&b, &a);

	//  if(bn_is_zero(&b)){
	//  	EMSG("Failed in computing elliptic curve cryptographic key derivation!");
	//  	return TEE_ERROR_BAD_PARAMETERS; //choose another type of errot
	// }

	// compute b = b mod curve->order by computing  b >= curve->order ? b - curve->order : b.
	// assumes b partly reduced, guarantees b fully reduced.
	// bn_mod(&b, &curve->order);

	// bn_write_be(&b, child_sk);
	// printf("child_sk b(256): 0x%x \n", child_sk);
	// 

	// printf("parent_ir before:");
	// print_uint8(ir, 32); 
	// memset(&c2, 0, sizeof(c2));
	// TEE_MemMove(c2+32, ir, 32);
	// printf("parent_ir after:");
	// print_uint8(&c2, 64); 






	//version0
	// printf("parent_sk before:");
	// print_uint8(parent_sk, 32); 
	// from_big_endian(parent_sk, 32, &a2);
	// printf("parent_sk after:");
	// print_uint8(&a2, 64); 

	// printf("parent_il before:");
	// print_uint8(il, 32); 
	// from_big_endian(il, 32, &b2);
	// printf("parent_il after:");
	// print_uint8(&b2, 64); 

	//from_big_endian256to512(&curve->order, &order); //36 is 32bytes + 4bytes, in point of view they want to specify 32+2, but they implement 32+4
		//if(!bn_is_less512(&b2,&order)){    /// if il >= order
	//	return TEE_ERROR_BAD_PARAMETERS;
	//}
 
	//if(bn_is_equal512(&add_res,&order)){ /// if add_res == order
	//	return TEE_ERROR_BAD_PARAMETERS;
	//}

	//if(!bn_is_less512(&add_res,&order)){
	//	bn_subtract512(&add_res,&order);//joao //check
	//}

	//to_big_endian(&add_res,32,child_sk); //check
	//printf("child_sk b2(512): 0x%x \n", child_sk);
	//print_uint8(child_sk, 32);

	// printf("parent_sk before:");
	// print_uint8(&parent_sk, 32);
	// print_uint8(parent_sk, 32); 
	// memset(&a3, 0, sizeof(a3));
	// TEE_MemMove(a3+32, parent_sk, 32);
	// printf("parent_sk after:");
	// print_uint8(&a3, 64); 

	// memset(&b3, 0, sizeof(b3));
	// TEE_MemMove(b3+32, il, 32);
	// printf("parent_il:");
	// print_uint8(&b3, 64); 

	// memset(&order, 0, sizeof(order));
	// from_big_endian33to1(&curve->order, &order);

	// bn_add8(&b3,&a3, &add_res2); //joao

	// if(!bn_is_less8(&b3,&order)){    /// if il >= order
	// 	return TEE_ERROR_BAD_PARAMETERS;
	// }

	// if(bn_is_equal8(&add_res2,&order)){    /// if res >= order
	// 	return TEE_ERROR_BAD_PARAMETERS;
	// }
	
	// if(!bn_is_less8(&add_res2,&order)){    /// if il >= order
	// 	bn_subtract512(&add_res2,&order);//joao //check
	// }	

	// printf("child_sk64 END:");
	// print_uint8(add_res2, 64);

	// TEE_MemMove(child_sk, add_res2+32, 32);
	// printf("child_sk END:");
	// print_uint8(child_sk, 32);
	// TEE_MemMove(child_chaincode, ir, 32);
	// printf("child_chaincode END:");
	// print_uint8(child_chaincode, 32);

	// return TEE_SUCCESS;
}


TEE_Result hdnode_public_ckd(uint8_t* parent_sk, uint8_t* parent_chaincode, uint32_t i, uint8_t* child_pk_x, uint8_t* child_pk_y)
{
	uint8_t child_sk[32];
	uint8_t child_pk[65];
	uint8_t child_chaincode[32];
	const ecdsa_curve* curve = &secp256k1;

	hdnode_private_ckd(parent_sk, parent_chaincode, i, child_sk, child_chaincode);
	
	/*computes extended public key based on extended private key*/
	ecdsa_get_public_key65(curve, child_sk, child_pk);

	TEE_MemMove(child_pk_x, child_pk+1, 32);
	TEE_MemMove(child_pk_y, child_pk+33, 32);

	// printf("i: %u\n", i);

	printf("\nChild_sk:");
	print_uint8(child_sk, 32);
	printf("Child_cc:");
	print_uint8(child_chaincode, 32);

	printf("\nChild_pk_x:");
	print_uint8(child_pk_x, 32);
	printf("Child_pk_y:");
	print_uint8(child_pk_y, 32);
	printf("\n");

	return TEE_SUCCESS;
}
