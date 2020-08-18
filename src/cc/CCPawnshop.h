/******************************************************************************
 * Copyright © 2014-2020 The SuperNET Developers.                             *
 *                                                                            *
 * See the AUTHORS, DEVELOPER-AGREEMENT and LICENSE files at                  *
 * the top-level directory of this distribution for the individual copyright  *
 * holder information and the developer policies on copyright and licensing.  *
 *                                                                            *
 * Unless otherwise agreed in a custom licensing agreement, no part of the    *
 * SuperNET software, including this file may be copied, modified, propagated *
 * or distributed except according to the terms contained in the LICENSE file *
 *                                                                            *
 * Removal or modification of this copyright notice is prohibited.            *
 *                                                                            *
 ******************************************************************************/

#ifndef CC_PAWNSHOP_H
#define CC_PAWNSHOP_H

/*
The Pawnshop CC enables private token exchanges between 2 pubkeys (since the Assets CC is for public orders only) using a dual 1of2 address setup, 
one for coins and the other for tokens.
This design allows for three main use cases:
	
	* Standard exchange of coins and tokens between two parties. 
	* Settlement of agreements created with Agreements CC (either milestone or final settlements, using the agreementunlock rpc and the PTF_REQUIREUNLOCK flag)
	* Secured loans with borrower's tokens used as collateral (TBD, NOT IMPLEMENTED YET)

The following RPCs are a proof of concept and will be implemented at some point in the future:
	pawnshopschedule
		Creates a loan repayment schedule. This must be executed by the lender (coin supplier) pubkey before the coins in the coin 1of2 address can be borrowed.
	pawnshopborrow
		Sends coins from coin 1of2 address as normal output to the borrower's wallet, and initiates the loan. Only available to the borrower (token supplier) pubkey. Requires
		definition of a scheduletxid created from the pawnshopschedule rpc - this will be the "official" repayment schedule of the loan.
	pawnshopseize
		Claims the tokens from the token 1of2 address and sends them to lender's wallet. Only available to the lender (coin supplier) pubkey, if there is a loan in progress and
		the due date specified in the loan repayment schedule has passed.
	pawnshopredeem
		Claims specified amount of coins from coin 1of2 address and sends them to lender's wallet, as long as the amount doesn't exceed what is specified in the loan repayment 
		schedule for that specific date. (more details on how this will work TBD)
*/

#define PAWNSHOPCC_VERSION 1
#define PAWNSHOPCC_MAXVINS 500
#define CC_TXFEE 10000
#define CC_MARKER_VALUE 10000
#define CC_BATON_VALUE 10000

#include "CCinclude.h"

enum EPawnshopTypeFlags
{
	PTF_REQUIREUNLOCK = 1, // 0b00000001 - when set, pawnshop instance requires usage of agreementunlock before pawnshopexchange can be used. Requires valid agreementtxid defined.
	PTF_NOLOAN = 2, // 0b00000010 - when set, any loan-related rpcs like pawnshopborrow are disabled. Currently mandatory as loan functionality isn't built yet.
	PTF_NOTRADE = 4, // 0b00000100 - when set, disables pawnshopexchange before a loan is initiated. Currently always disabled.
	// more flags may be introduced in the future
};

enum EPawnshopInputsFlags
{
	PIF_COINS = 0,
	PIF_TOKENS = 1,
};

/*************************************************
* EncodePawnshopCreateOpRet
*
* Description: Encodes op_return data required for a valid Pawnshop
*              instance creation transaction into a CScript object, and
*              appends the 'c' function id.
*
* Arguments:   - uint8_t version: version of data structure
*              - std::string name: name of the instance (max 32 chars)
*              - CPubKey tokensupplier: token supplier's pubkey object
*              - CPubKey coinsupplier: coin supplier's pubkey object
*              - uint32_t pawnshopflags: optional flags for instance
*              - uint256 tokenid: id of token being offered
*              - int64_t numtokens: amount of tokens required
*              - int64_t numcoins: amount of coins required
*              - uint256 agreementtxid: agreement to link the Pawnshop
*                instance to
*
* Returns the encoded CScript object.
**************************************************/
CScript EncodePawnshopCreateOpRet(uint8_t version,std::string name,CPubKey tokensupplier,CPubKey coinsupplier,uint32_t pawnshopflags,uint256 tokenid,int64_t numtokens,int64_t numcoins,uint256 agreementtxid);

/*************************************************
* DecodePawnshopCreateOpRet
*
* Description: Decodes op_return data required for a valid Pawnshop 
*              instance creation transaction (containing 'c' function id)
*              from a CScript object.
*
* Arguments:   - CScript scriptPubKey: op_return data object
*              - uint8_t &version: version of data structure
*              - std::string &name: name of the instance (max 32 chars)
*              - CPubKey &tokensupplier: token supplier's pubkey object
*              - CPubKey &coinsupplier: coin supplier's pubkey object
*              - uint32_t &pawnshopflags: optional flags for instance
*              - uint256 &tokenid: id of token being offered
*              - int64_t &numtokens: amount of tokens required
*              - int64_t &numcoins: amount of coins required
*              - uint256 &agreementtxid: agreement to link the Pawnshop
*                instance to
* 
* Returns the function id found within the CScript object. If data malformed
* or function is invalid, returns (uint8_t)0.
**************************************************/
uint8_t DecodePawnshopCreateOpRet(CScript scriptPubKey,uint8_t &version,std::string &name,CPubKey &tokensupplier,CPubKey &coinsupplier,uint32_t &pawnshopflags,uint256 &tokenid,int64_t &numtokens,int64_t &numcoins,uint256 &agreementtxid);

// not implemented yet //
CScript EncodePawnshopScheduleOpRet(uint8_t version,uint256 createtxid,int64_t principal,uint64_t duedate,bool bRelative);
uint8_t DecodePawnshopScheduleOpRet(CScript scriptPubKey,uint8_t &version,uint256 &createtxid,int64_t &principal,uint64_t &duedate,bool &bRelative);
// not implemented yet //

/*************************************************
* EncodePawnshopOpRet
*
* Description: Encodes op_return data required for a generic Pawnshop
*              transaction into a CScript object, and appends the 
*              specified function id.
*
* Arguments:   - uint8_t funcid: function id of the transaction
*              - uint8_t version: version of data structure
*              - uint256 createtxid: id of the relevant Pawnshop instance
*              - uint256 tokenid: id of token of the instance
*              - CPubKey tokensupplier: token supplier's pubkey object
*              - CPubKey coinsupplier: coin supplier's pubkey object
*
* Returns the encoded CScript object.
**************************************************/
CScript EncodePawnshopOpRet(uint8_t funcid,uint8_t version,uint256 createtxid,uint256 tokenid,CPubKey tokensupplier,CPubKey coinsupplier);

/*************************************************
* DecodePawnshopOpRet
*
* Description: Decodes op_return data required for a generic Pawnshop 
*              transaction from a CScript object.
*
* Arguments:   - CScript scriptPubKey: op_return data object
*              - uint8_t &version: version of data structure
*              - uint256 &createtxid: id of the relevant Pawnshop instance
*              - uint256 &tokenid: id of token of the instance
* 
* Returns the function id found within the CScript object. If data malformed
* or function is invalid, returns (uint8_t)0.
**************************************************/
uint8_t DecodePawnshopOpRet(const CScript scriptPubKey,uint8_t &version,uint256 &createtxid,uint256 &tokenid);

/*************************************************
* PawnshopValidate
*
* Description: Main validation code of the Pawnshop CC. Is triggered when
*              a transaction spending one or more CC outputs marked with the
*              EVAL_PAWNSHOP eval code is broadcasted to the node network
*              or when a block containing such a transaction is received by a
*              node.
*
* Arguments:   - struct CCcontract_info *cp: CCcontract_info object with 
*                Pawnshop CC variables (global CC address, global private
*                key, etc.)
*              - Eval* eval: pointer to the CC dispatching object
*              - const CTransaction &tx: the transaction to be validated
*              - uint32_t nIn: not used in validation code
* 
* Returns true if transaction is valid, otherwise false or calls eval->Invalid().
**************************************************/
bool PawnshopValidate(struct CCcontract_info *cp, Eval* eval, const CTransaction &tx, uint32_t nIn);

/*************************************************
* IsPawnshopvout
*
* Description: Helper function that determines if the specified transaction
*              CC output is marked with the EVAL_PAWNSHOP eval code.
*
* Arguments:   - struct CCcontract_info *cp: CCcontract_info object with 
*                Pawnshop CC variables (global CC address, global private
*                key, etc.)
*              - const CTransaction& tx: the transaction to check for outputs
*              - bool mode: whether to check for a token or coin output
*              - CPubKey tokensupplier: token supplier's pubkey object
*              - CPubKey coinsupplier: coin supplier's pubkey object
*              - int32_t v: the index of the output
* 
* Returns value of the output if it is a Pawnshop output, otherwise
* 0.
**************************************************/
int64_t IsPawnshopvout(struct CCcontract_info *cp,const CTransaction& tx,bool mode,CPubKey tokensupplier,CPubKey coinsupplier,int32_t v);

/*************************************************
* PawnshopExactAmounts
*
* Description: Helper function that checks if a Pawnshop transaction's inputs
*              match its outputs (all coins & tokens from the Pawnshop escrow
*              are sent to their correct destinations). In addition, it will
*              check if the Pawnshop creation transaction IDs in all relevant
*              op_return data are matched, to prevent direct transfer of
*              funds from one Pawnshop instance to another.
*
* Arguments:   - struct CCcontract_info *cp: CCcontract_info object with 
*                Pawnshop CC variables (global CC address, global private
*                key, etc.)
*              - Eval* eval: pointer to the CC dispatching object
*              - const CTransaction& tx: the transaction to check
*              - int64_t &coininputs: total amount of coin CC inputs found 
*              - int64_t &tokeninputs: total amount of token CC inputs found 
* 
* Returns true if the inputs & outputs match, otherwise false or calls
* eval->Invalid().
**************************************************/
bool PawnshopExactAmounts(struct CCcontract_info *cp, Eval* eval, const CTransaction &tx, int64_t &coininputs, int64_t &tokeninputs);

/*************************************************
* GetLatestPawnshopTxid
*
* Description: Helper function that retrieves the id of the latest transaction
*              containing an unspent baton CC output, and its function id.
*              Currently only used for checking if the Pawnshop instance is
*              closed or not.
*
* Arguments:   - uint256 createtxid: the id of the Pawnshop instance creation
*                transaction
*              - uint256 &latesttxid: the id of the latest transaction
*              - uint8_t &funcid: the function id of the latest transaction
* 
* Returns true if the transaction id was retrieved successfully, otherwise
* false.
**************************************************/
bool GetLatestPawnshopTxid(uint256 createtxid, uint256 &latesttxid, uint8_t &funcid);

/*************************************************
* GetLatestPawnshopTxid
*
* Description: Helper function that attempts to find the id of the latest
*              transaction in the Pawnshop instance baton pattern with the
*              specified function id.
*              Currently only used for checking if the Pawnshop instance is
*              closed or not.
*
* Arguments:   - uint256 createtxid: the id of the Pawnshop instance creation
*                transaction
*              - uint256 type: the function id to check for
*              - uint8_t &typetxid: the id of the latest transaction with the
*                specified function id, if it exists
* 
* Returns true if the search was completed successfully, otherwise
* false.
**************************************************/
bool FindPawnshopTxidType(uint256 createtxid, uint8_t type, uint256 &typetxid);

/*************************************************
* CheckDepositUnlockCond
*
* Description: Helper function that checks if the specified Pawnshop instance's
*              linked agreement has its deposit unlocked and sent to the
*              instance's escrow.
*
* Arguments:   - uint256 createtxid: the id of the Pawnshop instance creation
*                transaction
* 
* Returns -1 if no agreementtxid in the instance is defined, or deposit is not
* unlocked yet, otherwise returns amount sent to the Pawnshop instance's escrow.
**************************************************/
int64_t CheckDepositUnlockCond(uint256 createtxid);

/*************************************************
* ValidatePawnshopCreateTx
*
* Description: Helper function that validates the contents of a Pawnshop
*              instance creation CTransaction object.
*              Used to validate Pawnshop instance creation transactions in 
*              RPC implementations and in validation code, since they are
*              not validated by default.
*
* Arguments:   - CTransaction opentx: the transaction object to validate
*              - std::string &CCerror: error message if the transaction
*                object is invalid
* 
* Returns true if the CTransaction object is valid, otherwise
* false.
**************************************************/
bool ValidatePawnshopCreateTx(CTransaction opentx, std::string &CCerror);

/*************************************************
* GetPawnshopInputs
*
* Description: Helper function that retrieves the current coin or token
*              balance of the relevant Pawnshop CC 1of2 address.
*              This function only returns valid unspent outputs and their
*              total value - it cannot add them to a mutable transaction.
*
* Arguments:   - struct CCcontract_info *cp: CCcontract_info object with 
*                Pawnshop CC variables (global CC address, global private
*                key, etc.)
*              - CTransaction createtx: the Pawnshop instance creation
*                transaction object
*              - bool mode: whether to check for token or coin inputs
*              - std::vector<std::pair<CAddressUnspentKey, CAddressUnspentValue> >
*                &validUnspentOutputs: the key-value pair vector of valid
*                unspent outputs found within the address
* 
* Returns amount of the current coin or token balance of the relevant
* Pawnshop CC 1of2 address.
**************************************************/
int64_t GetPawnshopInputs(struct CCcontract_info *cp,CTransaction createtx,bool mode,std::vector<std::pair<CAddressUnspentKey, CAddressUnspentValue> > &validUnspentOutputs);

/*************************************************
* AddPawnshopInputs
*
* Description: Helper function that retrieves the available coin or token
*              balance of the relevant Pawnshop CC 1of2 address.
*              This function adds valid unspent outputs found using 
*              GetPawnshopInputs to the specified mutable transaction.
*
* Arguments:   - struct CCcontract_info *cp: CCcontract_info object with 
*                Pawnshop CC variables (global CC address, global private
*                key, etc.)
*              - CMutableTransaction &mtx: the mutable transaction object,
*                typically a transaction currently bring constructed
*              - CTransaction createtx: the Pawnshop instance creation
*                transaction object
*              - bool mode: whether to check for token or coin inputs
*              - int32_t maxinputs: max amount of inputs to add before
*                returning early
* 
* Returns amount of the coin or token balance taken from the relevant
* Pawnshop CC 1of2 address.
**************************************************/
int64_t AddPawnshopInputs(struct CCcontract_info *cp,CMutableTransaction &mtx,CTransaction createtx,bool mode,int32_t maxinputs);

/*************************************************
* PawnshopCreate
*
* Description: Creates a pawnshop instance, with predefined required amounts
*              for coins and tokens of a specific tokenid. Participant pubkeys
*              are also defined here, as well as optional flags (see CCPawnshop.h
*              file). Can be executed by any pubkey (unless agreementtxid is defined,
*              in which case the pubkey must be a member of the agreement), however 
*              the pawnshop create txids will only be visible to members of the
*              instance if pawnshoplist or agreementsettlements rpcs are used.
*
* Arguments:   see the pawnshopcreate definition in the pawnshoprpc.cpp file
* 
* Returns UniValue object containing the status and raw transaction hex for
* broadcasting.
**************************************************/
UniValue PawnshopCreate(const CPubKey& pk,uint64_t txfee,std::string name,CPubKey tokensupplier,CPubKey coinsupplier,int64_t numcoins,uint256 tokenid,int64_t numtokens,uint32_t pawnshopflags,uint256 agreementtxid);

/*************************************************
* PawnshopFund
*
* Description: Sends specified amount of coins to the coin CC 1of2 address.
*              Can be done only by the coin supplier pubkey.
*
* Arguments:   see the pawnshopfund definition in the pawnshoprpc.cpp file
* 
* Returns UniValue object containing the status and raw transaction hex for
* broadcasting.
**************************************************/
UniValue PawnshopFund(const CPubKey& pk, uint64_t txfee, uint256 createtxid, int64_t amount);

/*************************************************
* PawnshopPledge
*
* Description: Sends a token amount equal to required token amount defined
*              in createtxid to the token CC 1of2 address. Can be done only
*              by the token supplier pubkey.
*
* Arguments:   see the pawnshoppledge definition in the pawnshoprpc.cpp file
* 
* Returns UniValue object containing the status and raw transaction hex for
* broadcasting.
**************************************************/
UniValue PawnshopPledge(const CPubKey& pk, uint64_t txfee, uint256 createtxid);

// not implemented yet //
UniValue PawnshopSchedule(const CPubKey& pk, uint64_t txfee, uint256 createtxid, int64_t principal, int64_t duedate, bool bRelative);
UniValue PawnshopBorrow(const CPubKey& pk, uint64_t txfee, uint256 createtxid, uint256 scheduletxid);
UniValue PawnshopSeize(const CPubKey& pk, uint64_t txfee, uint256 createtxid);
// not implemented yet //

/*************************************************
* PawnshopCancel
*
* Description: Cancels the specified pawnshop instance. pawnshopfund and
*              pawnshoppledge rpcs will be disabled for the specified createtxid,
*              and if the coin and/or token CC 1of2 address contain funds at the
*              time of cancel, they are refunded to the sender.
*              (TBD) Can also cancel loans in progress if executed by lender
*              (coin supplier) pubkey
*
* Arguments:   see the pawnshopcancel definition in the pawnshoprpc.cpp file
* 
* Returns UniValue object containing the status and raw transaction hex for
* broadcasting.
**************************************************/
UniValue PawnshopCancel(const CPubKey& pk, uint64_t txfee, uint256 createtxid);

/*************************************************
* PawnshopExchange
*
* Description: Closes the specified pawnshop instance by sending coins from
*              the coin 1of2 address to the token supplier, and tokens from
*              the token 1of2 address to the coin supplier.
*              pawnshopfund and pawnshoppledge rpcs will be disabled for the
*              specified createtxid. Only works if both addresses contain >=
*              required token/coin amounts.
*
* Arguments:   see the pawnshopexchange definition in the pawnshoprpc.cpp file
* 
* Returns UniValue object containing the status and raw transaction hex for
* broadcasting.
**************************************************/
UniValue PawnshopExchange(const CPubKey& pk, uint64_t txfee, uint256 createtxid);

/*************************************************
* PawnshopInfo
*
* Description: Returns information about a Pawnshop instance in the blockchain.
*
* Arguments:   - const CPubKey& pk: pubkey of the RPC submitter
*              - uint256 createtxid: Pawnshop instance creation transaction id.
* 
* Returns UniValue object containing the status and instance data in JSON format.
**************************************************/
UniValue PawnshopInfo(const CPubKey& pk, uint256 createtxid);

/*************************************************
* PawnshopList
*
* Description: Returns array of every Pawnshop instance creation transaction
*              id in the blockchain.
*
* Arguments:   - const CPubKey& pk: pubkey of the RPC submitter
* 
* Returns UniValue object containing the transaction id array.
**************************************************/
UniValue PawnshopList(const CPubKey& pk);

#endif
