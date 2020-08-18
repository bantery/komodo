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

/*
The Agreements CC enables anyone to create a blockchain representation of a bilateral agreement.
An agreement created using this CC features, among other things, the ability to store the checksum of off-chain contract documents (or an oracletxid) to prevent tampering, 
a two party approval protocol for actions such as updates, terminations, and a dispute resolution system which utilizes an arbitrator, a mutually agreed upon third party.
To create an active contract between two parties, the seller must first use the agreementcreate RPC to create a proposal and designate a client pubkey, 
which will need to execute the agreementaccept RPC and sign the proposal.
*/

#ifndef CC_AGREEMENTS_H
#define CC_AGREEMENTS_H

#include "CCinclude.h"

#define AGREEMENTCC_VERSION 1
#define CC_TXFEE 10000
#define CC_MARKER_VALUE 10000

/*************************************************
* DecodeAgreementOpRet
*
* Description: Decodes op_return data marked with EVAL_AGREEMENTS using
*              a decoder function corresponding to the found function id.
*
* Arguments:   - const CScript scriptPubKey: op_return data object
* 
* Returns function id of provided op_return data. If the function id is
* invalid or the data structure does not match specifications for its
* function id, returns (uint8_t)0. 
**************************************************/
uint8_t DecodeAgreementOpRet(const CScript scriptPubKey);

/*************************************************
* EncodeAgreementProposalOpRet
*
* Description: Encodes op_return data required for a valid agreement 
*              proposal transaction into a CScript object, and
*              appends the 'p' function id.
*
* Arguments:   - uint8_t version: version of data structure
*              - uint8_t proposaltype: type of proposal
*                Available types: 'p' (new agreement), 'u' (update
*                agreement), 'c' (close agreement)
*              - std::vector<uint8_t> srcpub: byte vector containing
*                pubkey of transaction creator
*              - std::vector<uint8_t> destpub: byte vector containing
*                pubkey of intended recipient of the proposal
*                (required for 'u' and 'c' proposal types)
*              - std::vector<uint8_t> arbitratorpk: byte vector 
*                containing pubkey of intended arbitrator of the 
*                proposed agreement
*              - int64_t payment: payment to srcpub required for
*                valid proposal acceptance
*              - int64_t arbitratorfee: payment to arbitratorpk
*                required for valid disputes of proposed
*                agreement
*              - int64_t depositval: deposit required
*              - uint256 datahash: field for arbitrary sha256 hash
*              - uint256 agreementtxid: transaction id of a valid 
*                agreement in the blockchain. Required for 'u' and
*                'c' proposal type data
*              - uint256 prevproposaltxid: transaction id of another
*                proposal to be superseded by this proposal
*              - std::string info: field for arbitrary data string
* 
* Returns the encoded CScript object.
**************************************************/
CScript EncodeAgreementProposalOpRet(uint8_t version, uint8_t proposaltype, std::vector<uint8_t> srcpub, std::vector<uint8_t> destpub, std::vector<uint8_t> arbitratorpk, int64_t payment, int64_t arbitratorfee, int64_t depositval, uint256 datahash, uint256 agreementtxid, uint256 prevproposaltxid, std::string info);

/*************************************************
* DecodeAgreementProposalOpRet
*
* Description: Decodes op_return data required for a valid agreement 
*              proposal transaction (containing 'p' function id)
*              from a CScript object.
*
* Arguments:   - CScript scriptPubKey: op_return data object
*              - uint8_t &version: version of data structure
*              - uint8_t &proposaltype: type of proposal
*                Available types: 'p' (new agreement), 'u' (update
*                agreement), 'c' (close agreement)
*              - std::vector<uint8_t> &srcpub: byte vector containing
*                pubkey of transaction creator
*              - std::vector<uint8_t> &destpub: byte vector containing
*                pubkey of intended recipient of the proposal
*                (required for 'u' and 'c' proposal types)
*              - std::vector<uint8_t> &arbitratorpk: byte vector 
*                containing pubkey of intended arbitrator of the 
*                proposed agreement
*              - int64_t &payment: payment to srcpub required for
*                valid proposal acceptance
*              - int64_t &arbitratorfee: payment to arbitratorpk
*                required for valid disputes of proposed
*                agreement
*              - int64_t &depositval: deposit required
*              - uint256 &datahash: field for arbitrary sha256 hash
*              - uint256 &agreementtxid: transaction id of a valid 
*                agreement in the blockchain. Required for 'u' and
*                'c' proposal type data
*              - uint256 &prevproposaltxid: transaction id of another
*                proposal to be superseded by this proposal
*              - std::string &info: field for arbitrary data string
* 
* Returns the function id found within the CScript object. If data malformed
* or function is invalid, returns (uint8_t)0.
**************************************************/
uint8_t DecodeAgreementProposalOpRet(CScript scriptPubKey, uint8_t &version, uint8_t &proposaltype, std::vector<uint8_t> &srcpub, std::vector<uint8_t> &destpub, std::vector<uint8_t> &arbitratorpk, int64_t &payment, int64_t &arbitratorfee, int64_t &depositval, uint256 &datahash, uint256 &agreementtxid, uint256 &prevproposaltxid, std::string &info);

/*************************************************
* EncodeAgreementProposalCloseOpRet
*
* Description: Encodes op_return data required for a valid agreement 
*              proposal closure transaction into a CScript object, and
*              appends the 't' function id.
*
* Arguments:   - uint8_t version: version of data structure
*              - uint256 proposaltxid: transaction id of proposal to
*                be closed
*              - std::vector<uint8_t> srcpub: byte vector containing
*                pubkey of transaction creator
*
* Returns the encoded CScript object.
**************************************************/
CScript EncodeAgreementProposalCloseOpRet(uint8_t version, uint256 proposaltxid, std::vector<uint8_t> srcpub);

/*************************************************
* DecodeAgreementProposalOpRet
*
* Description: Decodes op_return data required for a valid agreement 
*              proposal closure transaction (containing 't' function id)
*              from a CScript object.
*
* Arguments:   - CScript scriptPubKey: op_return data object
*              - uint8_t &version: version of data structure
*              - uint256 &proposaltxid: transaction id of proposal that
*                was closed
*              - std::vector<uint8_t> &srcpub: byte vector containing
*                pubkey of transaction creator
* 
* Returns the function id found within the CScript object. If data malformed
* or function is invalid, returns (uint8_t)0.
**************************************************/
uint8_t DecodeAgreementProposalCloseOpRet(CScript scriptPubKey, uint8_t &version, uint256 &proposaltxid, std::vector<uint8_t> &srcpub);

/*************************************************
* EncodeAgreementSigningOpRet
*
* Description: Encodes op_return data required for a valid agreement 
*              proposal signing transaction into a CScript object, and
*              appends the 'c' function id.
*
* Arguments:   - uint8_t version: version of data structure
*              - uint256 proposaltxid: transaction id of proposal to
*                be accepted
*
* Returns the encoded CScript object.
**************************************************/
CScript EncodeAgreementSigningOpRet(uint8_t version, uint256 proposaltxid);

/*************************************************
* DecodeAgreementSigningOpRet
*
* Description: Decodes op_return data required for a valid agreement 
*              proposal closure transaction (containing 'c' function id)
*              from a CScript object.
*
* Arguments:   - CScript scriptPubKey: op_return data object
*              - uint8_t &version: version of data structure
*              - uint256 &proposaltxid: transaction id of proposal that
*                was accepted
* 
* Returns the function id found within the CScript object. If data malformed
* or function is invalid, returns (uint8_t)0.
**************************************************/
uint8_t DecodeAgreementSigningOpRet(CScript scriptPubKey, uint8_t &version, uint256 &proposaltxid);

/*************************************************
* EncodeAgreementUpdateOpRet
*
* Description: Encodes op_return data required for a valid agreement 
*              update transaction into a CScript object, and
*              appends the 'u' function id.
*
* Arguments:   - uint8_t version: version of data structure
*              - uint256 agreementtxid: transaction id of agreement to
*                be updated
*              - uint256 proposaltxid: transaction id of update proposal to
*                be accepted
*
* Returns the encoded CScript object.
**************************************************/
CScript EncodeAgreementUpdateOpRet(uint8_t version, uint256 agreementtxid, uint256 proposaltxid);

/*************************************************
* DecodeAgreementUpdateOpRet
*
* Description: Decodes op_return data required for a valid agreement 
*              update transaction (containing 'u' function id)
*              from a CScript object.
*
* Arguments:   - CScript scriptPubKey: op_return data object
*              - uint8_t &version: version of data structure
*              - uint256 &agreementtxid: transaction id of agreement that
*                was updated
*              - uint256 &proposaltxid: transaction id of update proposal that
*                was accepted
* 
* Returns the function id found within the CScript object. If data malformed
* or function is invalid, returns (uint8_t)0.
**************************************************/
uint8_t DecodeAgreementUpdateOpRet(CScript scriptPubKey, uint8_t &version, uint256 &agreementtxid, uint256 &proposaltxid);

/*************************************************
* EncodeAgreementCloseOpRet
*
* Description: Encodes op_return data required for a valid agreement 
*              closure transaction into a CScript object, and
*              appends the 's' function id.
*
* Arguments:   - uint8_t version: version of data structure
*              - uint256 agreementtxid: transaction id of agreement to
*                be closed
*              - uint256 proposaltxid: transaction id of closure proposal to
*                be accepted
*
* Returns the encoded CScript object.
**************************************************/
CScript EncodeAgreementCloseOpRet(uint8_t version, uint256 agreementtxid, uint256 proposaltxid);

/*************************************************
* DecodeAgreementCloseOpRet
*
* Description: Decodes op_return data required for a valid agreement 
*              closure transaction (containing 's' function id)
*              from a CScript object.
*
* Arguments:   - CScript scriptPubKey: op_return data object
*              - uint8_t &version: version of data structure
*              - uint256 &agreementtxid: transaction id of agreement that
*                was closed
*              - uint256 &proposaltxid: transaction id of closure proposal that
*                was accepted
* 
* Returns the function id found within the CScript object. If data malformed
* or function is invalid, returns (uint8_t)0.
**************************************************/
uint8_t DecodeAgreementCloseOpRet(CScript scriptPubKey, uint8_t &version, uint256 &agreementtxid, uint256 &proposaltxid);

/*************************************************
* EncodeAgreementDisputeOpRet
*
* Description: Encodes op_return data required for a valid agreement 
*              dispute transaction into a CScript object, and
*              appends the 'd' function id.
*
* Arguments:   - uint8_t version: version of data structure
*              - uint256 agreementtxid: transaction id of agreement in 
*                dispute
*              - std::vector<uint8_t> srcpub: byte vector containing
*                pubkey of transaction creator
*              - uint256 datahash: field for arbitrary sha256 hash
*
* Returns the encoded CScript object.
**************************************************/
CScript EncodeAgreementDisputeOpRet(uint8_t version, uint256 agreementtxid, std::vector<uint8_t> srcpub, uint256 datahash);

/*************************************************
* DecodeAgreementDisputeOpRet
*
* Description: Decodes op_return data required for a valid agreement 
*              dispute transaction (containing 'd' function id)
*              from a CScript object.
*
* Arguments:   - CScript scriptPubKey: op_return data object
*              - uint8_t &version: version of data structure
*              - uint256 &agreementtxid: transaction id of agreement in 
*                dispute
*              - std::vector<uint8_t> &srcpub: byte vector containing
*                pubkey of transaction creator
*              - uint256 &datahash: field for arbitrary sha256 hash
* 
* Returns the function id found within the CScript object. If data malformed
* or function is invalid, returns (uint8_t)0.
**************************************************/
uint8_t DecodeAgreementDisputeOpRet(CScript scriptPubKey, uint8_t &version, uint256 &agreementtxid, std::vector<uint8_t> &srcpub, uint256 &datahash);

/*************************************************
* EncodeAgreementDisputeResolveOpRet
*
* Description: Encodes op_return data required for a valid agreement 
*              dispute resolution transaction into a CScript object, and
*              appends the 'r' function id.
*
* Arguments:   - uint8_t version: version of data structure
*              - uint256 agreementtxid: transaction id of agreement in
*                dispute to be resolved
*              - std::vector<uint8_t> rewardedpubkey: byte vector containing
*                pubkey of the dispute "winner", according to the arbitrator
*
* Returns the encoded CScript object.
**************************************************/
CScript EncodeAgreementDisputeResolveOpRet(uint8_t version, uint256 agreementtxid, std::vector<uint8_t> rewardedpubkey);

/*************************************************
* DecodeAgreementDisputeResolveOpRet
*
* Description: Decodes op_return data required for a valid agreement 
*              dispute resolution transaction (containing 'u' function id)
*              from a CScript object.
*
* Arguments:   - CScript scriptPubKey: op_return data object
*              - uint8_t &version: version of data structure
*              - uint256 &agreementtxid: transaction id of agreement in
*                dispute that was resolved
*              - std::vector<uint8_t> &rewardedpubkey: byte vector containing
*                pubkey of the dispute "winner", according to the arbitrator
* 
* Returns the function id found within the CScript object. If data malformed
* or function is invalid, returns (uint8_t)0.
**************************************************/
uint8_t DecodeAgreementDisputeResolveOpRet(CScript scriptPubKey, uint8_t &version, uint256 &agreementtxid, std::vector<uint8_t> &rewardedpubkey);

/*************************************************
* EncodeAgreementUnlockOpRet
*
* Description: Encodes op_return data required for a valid agreement 
*              unlock transaction into a CScript object, and
*              appends the 'n' function id.
*
* Arguments:   - uint8_t version: version of data structure
*              - uint256 agreementtxid: transaction id of agreement to
*                have its deposit unlocked
*              - uint256 pawnshoptxid: transaction id of the Pawnshop
*                instance where the deposit funds are sent
*
* Returns the encoded CScript object.
**************************************************/
CScript EncodeAgreementUnlockOpRet(uint8_t version, uint256 agreementtxid, uint256 pawnshoptxid);

/*************************************************
* DecodeAgreementUnlockOpRet
*
* Description: Decodes op_return data required for a valid agreement 
*              update transaction (containing 'n' function id)
*              from a CScript object.
*
* Arguments:   - CScript scriptPubKey: op_return data object
*              - uint8_t &version: version of data structure
*              - uint256 &agreementtxid: transaction id of agreement that
*                had its deposit unlocked
*              - uint256 &pawnshoptxid: transaction id of the Pawnshop
*                instance where the deposit funds are sent
* 
* Returns the function id found within the CScript object. If data malformed
* or function is invalid, returns (uint8_t)0.
**************************************************/
uint8_t DecodeAgreementUnlockOpRet(CScript scriptPubKey, uint8_t &version, uint256 &agreementtxid, uint256 &pawnshoptxid);

/*************************************************
* AgreementsValidate
*
* Description: Main validation code of the Agreements CC. Is triggered when
*              a transaction spending one or more CC outputs marked with the
*              EVAL_AGREEMENTS eval code is broadcasted to the node network
*              or when a block containing such a transaction is received by a
*              node.
*
* Arguments:   - struct CCcontract_info *cp: CCcontract_info object with 
*                Agreements CC variables (global CC address, global private
*                key, etc.)
*              - Eval* eval: pointer to the CC dispatching object
*              - const CTransaction &tx: the transaction to be validated
*              - uint32_t nIn: not used in validation code
* 
* Returns true if transaction is valid, otherwise false or calls eval->Invalid().
**************************************************/
bool AgreementsValidate(struct CCcontract_info *cp, Eval* eval, const CTransaction &tx, uint32_t nIn);

/*************************************************
* GetAcceptedProposalOpRet
*
* Description: Helper function that extracts the CScript op_return data object
*              from the proposal transaction CTransaction object.
*              Used to help extract required data for validating proposal
*              acceptance transactions.
*
* Arguments:   - CTransaction tx: proposal transaction object
*              - uint256 &proposaltxid: id of the proposal transaction
*              - CScript &opret: the CScript op_return data object
* 
* Returns true if the CScript object was retrieved successfully, otherwise
* false.
**************************************************/
bool GetAcceptedProposalOpRet(CTransaction tx, uint256 &proposaltxid, CScript &opret);

/*************************************************
* ValidateProposalOpRet
*
* Description: Helper function that validates a CScript op_return data object
*              of a proposal transaction.
*              Used to validate proposal submit transactions in RPC
*              implementations and in validation code, since they are not
*              validated by default.
*
* Arguments:   - CScript opret: the CScript op_return data object
*              - std::string &CCerror: error message if the data
*                object is invalid
* 
* Returns true if the CScript object is valid, otherwise
* false.
**************************************************/
bool ValidateProposalOpRet(CScript opret, std::string &CCerror);

/*************************************************
* CompareProposals
*
* Description: Helper function that validates a CScript op_return data object
*              of a proposal transaction in reference to another existing
*              proposal on the blockchain.
*              Used to validate proposal transactions that amend other
*              proposals in RPC implementations and in validation code.
*
* Arguments:   - CScript proposalopret: the CScript op_return data object
*              - uint256 refproposaltxid: the id of an existing proposal
*                transaction for comparison
*              - std::string &CCerror: error message if the data
*                object is invalid
* 
* Returns true if the CScript object is valid, otherwise
* false.
**************************************************/
bool CompareProposals(CScript proposalopret, uint256 refproposaltxid, std::string &CCerror);

/*************************************************
* IsProposalSpent
*
* Description: Helper function that checks if the baton CC output of the
*              specified proposal has been spent by another transaction.
*
* Arguments:   - uint256 proposaltxid: the id of the proposal transaction
*              - uint256 &spendingtxid: the id of the transaction that spent
*                the baton, if it exists
*              - uint8_t &spendingfuncid: the function id of the spending
*                transaction, if it exists
* 
* Returns true if the proposal baton is spent, otherwise
* false.
**************************************************/
bool IsProposalSpent(uint256 proposaltxid, uint256 &spendingtxid, uint8_t &spendingfuncid);

/*************************************************
* GetAgreementInitialData
*
* Description: Helper function that retrieves static data corresponding
*              to the specified id of an agreement transaction.
*
* Arguments:   - uint256 agreementtxid: the id of the agreement transaction
*              - uint256 &proposaltxid: the id of the accepted proposal
*                transaction of the agreement
*              - std::vector<uint8_t> &sellerpk: byte vector of the seller
*                pubkey
*              - std::vector<uint8_t> &clientpk: byte vector of the client
*                pubkey
*              - std::vector<uint8_t> &arbitratorpk: byte vector of the
*                arbitrator pubkey
*              - int64_t &firstarbitratorfee: arbitrator fee amount
*                defined in the accepted proposal transaction
*              - int64_t &deposit: deposit amount allocated for the
*                agreement
*              - uint256 &firstdatahash: arbitrary sha256 hash defined
*                in the accepted proposal transaction
*              - uint256 &refagreementtxid: id of the reference (master)
*                agreement transaction
*              - std::string &firstinfo: arbitrary data string defined
*                in the accepted proposal transaction
* 
* Returns true if the data was retrieved successfully, otherwise
* false.
**************************************************/
bool GetAgreementInitialData(uint256 agreementtxid, uint256 &proposaltxid, std::vector<uint8_t> &sellerpk, std::vector<uint8_t> &clientpk, std::vector<uint8_t> &arbitratorpk, int64_t &firstarbitratorfee, int64_t &deposit, uint256 &firstdatahash, uint256 &refagreementtxid, std::string &firstinfo);

/*************************************************
* GetLatestAgreementUpdate
*
* Description: Helper function that retrieves the id of the latest transaction
*              containing an unspent baton CC output, and its function id.
*
* Arguments:   - uint256 agreementtxid: the id of the agreement transaction
*              - uint256 &latesttxid: the id of the latest transaction
*              - uint8_t &funcid: the function id of the latest transaction
* 
* Returns true if the transaction id was retrieved successfully, otherwise
* false.
**************************************************/
bool GetLatestAgreementUpdate(uint256 agreementtxid, uint256 &latesttxid, uint8_t &funcid);

/*************************************************
* GetAgreementUpdateData
*
* Description: Helper function that retrieves the data related to a specific
*              agreement update transaction. Since the data is stored within
*              a CScript object of the accepted update proposal's transaction
*              rather than the update acceptance transaction itself, this
*              function can be used to retrieve the data easily with only an
*              agreement update id as a known argument.
*
* Arguments:   - uint256 updatetxid: the id of the agreement update transaction
*              - std::string &info: arbitrary data string defined in the
*                accepted update proposal transaction
*              - uint256 &datahash: arbitrary sha256 hash defined in the
*                accepted update proposal transaction
*              - int64_t &arbitratorfee: arbitrator fee defined in the
*                accepted update proposal transaction
*              - int64_t &depositsplit: deposit split defined in the
*                accepted update proposal transaction. Only used if the update
*                proposal is of closure type ('s' function id)
*              - int64_t &revision: revision number of the specified agreement
*                update transaction
* 
* Returns true if the data was retrieved successfully, otherwise false.
**************************************************/
void GetAgreementUpdateData(uint256 updatetxid, std::string &info, uint256 &datahash, int64_t &arbitratorfee, int64_t &depositsplit, int64_t &revision);

/*************************************************
* AgreementCreate
*
* Description: Creates or updates an agreement proposal, with an optional 
*              prepayment that will be required for its acceptance by the other
*              party. The pubkey that executes this function will always be the seller
*              in the resulting contract. 
*              The client pubkey may or may not be specified, but the resulting
*              proposal must have a designated client to be able to be accepted.
*              The arbitrator pubkey may or may not be specified. If an arbitrator
*              is included, an arbitrator fee must also be specified, and the deposit
*              will be controlled by the arbitrator when a dispute occurs.
*
* Arguments:   - const CPubKey& pk: pubkey of transaction creator / submitter
*              - uint64_t txfee: transaction fee in satoshis
*              - std::string info: arbitrary info string (up to 2048 chars)
*              - uint256 datahash: arbitrary sha256 hash
*              - std::vector<uint8_t> destpub: pubkey of proposal recipient,
*                can be set to 0 for no recipient
*              - std::vector<uint8_t> arbitrator: pubkey of proposed arbitrator,
*                can be set to 0 for no arbitrator
*              - int64_t payment: payment required to accept this proposal
*              - int64 arbitratorfee: fee required for creating disputes for
*                proposed agreement to the arbitrator
*              - int64_t deposit: deposit required for this agreement
*              - uint256 prevproposaltxid: transaction id of proposal to be
*                superseded by this proposal. Can be set to zeroid to disable
*                this feature
*              - uint256 refagreementtxid: transaction id of valid agreement
*                in the blockchain (must have at least one shared member pubkey)
*                to be set as a master agreement. Can be set to zeroid to
*                disable this feature
* 
* Returns UniValue object containing the status and raw transaction hex for
* broadcasting.
**************************************************/
UniValue AgreementCreate(const CPubKey& pk, uint64_t txfee, std::string info, uint256 datahash, std::vector<uint8_t> destpub, std::vector<uint8_t> arbitrator, int64_t payment, int64_t arbitratorfee, int64_t deposit, uint256 prevproposaltxid, uint256 refagreementtxid);

/*************************************************
* AgreementUpdate
*
* Description: Opens a request to update an existing agreement. This function
*              must be executed by one of the members of the agreement, not 
*              including the arbitrator.
*
* Arguments:   - const CPubKey& pk: pubkey of transaction creator / submitter
*              - uint64_t txfee: transaction fee in satoshis
*              - uint256 agreementtxid: transaction id of agreement to be
*                updated
*              - std::string info: arbitrary info string (up to 2048 chars)
*              - uint256 datahash: arbitrary sha256 hash
*              - int64_t payment: payment required to accept this proposal
*              - uint256 prevproposaltxid: transaction id of proposal to be
*                superseded by this proposal. Can be set to zeroid to disable
*                this feature
*              - int64_t newarbitratorfee: new fee required for creating disputes
*                for proposed agreement to the arbitrator
* 
* Returns UniValue object containing the status and raw transaction hex for
* broadcasting.
**************************************************/
UniValue AgreementUpdate(const CPubKey& pk, uint64_t txfee, uint256 agreementtxid, std::string info, uint256 datahash, int64_t payment, uint256 prevproposaltxid, int64_t newarbitratorfee);

/*************************************************
* AgreementClose
*
* Description: Opens a request to permanently close an existing agreement. This 
*              function must be executed by one of the members of the agreement, 
*              not including the arbitrator.
*              If the agreement has a deposit locked in, the acceptance of this
*              request will split it between the two members. The deposit split
*              can be adjusted by using a parameter in this function.
*
* Arguments:   - const CPubKey& pk: pubkey of transaction creator / submitter
*              - uint64_t txfee: transaction fee in satoshis
*              - uint256 agreementtxid: transaction id of agreement to be
*                updated
*              - std::string info: arbitrary info string (up to 2048 chars)
*              - uint256 datahash: arbitrary sha256 hash
*              - int64_t depositcut: amount of the deposit offered to the
*                recipient of this proposal, the rest goes to the sender
*              - int64_t payment: payment required to accept this proposal
*              - uint256 prevproposaltxid: transaction id of proposal to be
*                superseded by this proposal. Can be set to zeroid to disable
*                this feature  
* 
* Returns UniValue object containing the status and raw transaction hex for
* broadcasting.
**************************************************/
UniValue AgreementClose(const CPubKey& pk, uint64_t txfee, uint256 agreementtxid, std::string info, uint256 datahash, int64_t depositcut, int64_t payment, uint256 prevproposaltxid);

/*************************************************
* AgreementStopProposal
*
* Description: Closes the specified agreement proposal/request. The proposal 
*              txid specified must be of an active proposal – if the txid has
*              been updated or closed by another txid down the proposal's update
*              chain, the function will not work.
*              This function can be executed by the proposal's initiator or,
*              if they exist, the receiver.
*
* Arguments:   - const CPubKey& pk: pubkey of transaction creator / submitter
*              - uint64_t txfee: transaction fee in satoshis
*              - uint256 proposaltxid: transaction id of proposal to be
*                closed
* 
* Returns UniValue object containing the status and raw transaction hex for
* broadcasting.
**************************************************/
UniValue AgreementStopProposal(const CPubKey& pk, uint64_t txfee, uint256 proposaltxid);

/*************************************************
* AgreementAccept
*
* Description: Accepts and signs the specified agreement proposal/request.
*              The proposal txid specified must be of an active proposal – if the
*              txid has been updated or closed by another txid down the proposal's
*              update chain, the function will not work. 
*              This function can only be executed by the proposal's receiver.
*
* Arguments:   - const CPubKey& pk: pubkey of transaction creator / submitter
*              - uint64_t txfee: transaction fee in satoshis
*              - uint256 proposaltxid: transaction id of proposal to be
*                accepted
* 
* Returns UniValue object containing the status and raw transaction hex for
* broadcasting.
**************************************************/
UniValue AgreementAccept(const CPubKey& pk, uint64_t txfee, uint256 proposaltxid);

/*************************************************
* AgreementDispute
*
* Description: Opens a new agreement dispute. This function must be executed by
*              one of the members of the agreement, not including the arbitrator. 
*              This transaction will require paying the arbitrator fee that was
*              specified at the agreement's proposal stage.
*              Only available if the agreement has an arbitrator pubkey specified.
*
* Arguments:   - const CPubKey& pk: pubkey of transaction creator / submitter
*              - uint64_t txfee: transaction fee in satoshis
*              - uint256 agreementtxid: transaction id of agreement to be
*                disputed
*              - uint256 datahash: arbitrary sha256 hash
* 
* Returns UniValue object containing the status and raw transaction hex for
* broadcasting.
**************************************************/
UniValue AgreementDispute(const CPubKey& pk, uint64_t txfee, uint256 agreementtxid, uint256 datahash);

/*************************************************
* AgreementResolve
*
* Description: Resolves an existing contract dispute. This function can only be
*              executed by the arbitrator, who can pay out the deposit
*              to any member of the agreement as they see fit.
*
* Arguments:   - const CPubKey& pk: pubkey of transaction creator / submitter
*              - uint64_t txfee: transaction fee in satoshis
*              - uint256 agreementtxid: transaction id of agreement to be
*                resolved
*              - std::vector<uint8_t> rewardedpubkey: pubkey to send the
*                deposit to
* 
* Returns UniValue object containing the status and raw transaction hex for
* broadcasting.
**************************************************/
UniValue AgreementResolve(const CPubKey& pk, uint64_t txfee, uint256 agreementtxid, std::vector<uint8_t> rewardedpubkey);

/*************************************************
* AgreementUnlock
*
* Description: Sends the agreement completion marker / deposit to the specified 
*              Pawnshop transaction id address. This function can only be executed
*              if Pawnshop CC is enabled, and the pawnshop has the agreement defined
*              and deposit unlocking enabled.
*              The pawnshop escrow must have enough tokens and coins (including the
*              deposit amount) to satisfy the pawnshop requirements.
*              A specific amount is sent to the pawnshop, so that the pawnshop coin
*              balance matches the required amount of coins. The remainder is sent
*              back to the agreement client.
*              This RPC can only be executed by a member of the agreement (not
*              including the arbitrator).
*
* Arguments:   - const CPubKey& pk: pubkey of transaction creator / submitter
*              - uint64_t txfee: transaction fee in satoshis
*              - uint256 agreementtxid: transaction id of agreement to be
*                resolved
*              - uint256 agreementtxid: transaction id of the Pawnshop instance
* 
* Returns UniValue object containing the status and raw transaction hex for
* broadcasting.
**************************************************/
UniValue AgreementUnlock(const CPubKey& pk, uint64_t txfee, uint256 agreementtxid, uint256 pawnshoptxid);

/*************************************************
* AgreementInfo
*
* Description: Returns information about any Agreements CC type
*              transaction id.
*
* Arguments:   - uint256 txid: Agreements CC type transaction id
* 
* Returns UniValue object containing the status and agreement data in JSON format.
**************************************************/
UniValue AgreementInfo(uint256 txid);

/*************************************************
* AgreementUpdateLog
*
* Description: Returns array of agreement update transaction ids for the
*              specified agreement transaction id.
*
* Arguments:   - uint256 agreementtxid: valid agreement transaction id
*              - int64_t samplenum: max amount of ids to retrieve
*              - bool backwards: if set, returns array sorted from latest id
*                to oldest. Otherwise, the returned array is sorted from oldest
*                id to latest.
* 
* Returns UniValue object containing the transaction id array.
**************************************************/
UniValue AgreementUpdateLog(uint256 agreementtxid, int64_t samplenum, bool backwards);

/*************************************************
* AgreementProposals
*
* Description: Returns three arrays (one for seller, client and arbitrator) of
*              agreement proposal transaction ids that the specified pubkey is
*              referenced in.
*
* Arguments:   - CPubKey pk: pubkey to check for
*              - uint256 agreementtxid: if set, filters out proposals unrelated to
*                this agreement transaction id
* 
* Returns UniValue object containing the sorted arrays in JSON format.
**************************************************/
UniValue AgreementProposals(CPubKey pk, uint256 agreementtxid);

/*************************************************
* AgreementSubcontracts
*
* Description: Returns array of agreement transaction ids that reference the
*              specified agreement transaction id as the master agreement.
*
* Arguments:   - uint256 agreementtxid: agreement transaction id to check for
* 
* Returns UniValue object containing the transaction id array.
**************************************************/
UniValue AgreementSubcontracts(uint256 agreementtxid);

/*************************************************
* AgreementInventory
*
* Description: Returns three arrays (one for seller, client and arbitrator) of
*              agreement transaction ids that the specified pubkey is a
*              member of.
*
* Arguments:   - CPubKey pk: pubkey to check for
* 
* Returns UniValue object containing the sorted arrays in JSON format.
**************************************************/
UniValue AgreementInventory(CPubKey pk);

/*************************************************
* AgreementSettlements
*
* Description: Returns array of Pawnshop transaction ids that reference
*              the specified agreement transaction id.
*              Note: Unlike the other functions documented here, this
*              function will only return non-empty arrays for members
*              of this agreement (not including the arbitrator).
*
* Arguments:   - const CPubKey& pk: pubkey of transaction creator / submitter
*              - uint256 agreementtxid: agreement transaction id to check for
*              - bool bActiveOnly: if set, filters out closed Pawnshop instances
* 
* Returns UniValue object containing the transaction id array.
**************************************************/
UniValue AgreementSettlements(const CPubKey& pk, uint256 agreementtxid, bool bActiveOnly);

/*************************************************
* AgreementList
*
* Description: Returns array of transaction ids of every valid agreement on
*              the blockchain.
*
* Arguments:   none
* 
* Returns UniValue object containing the transaction id array.
**************************************************/
UniValue AgreementList();

#endif
