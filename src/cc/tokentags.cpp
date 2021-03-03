/******************************************************************************
* Copyright © 2014-2021 The SuperNET Developers.                             *
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

#include "CCTokenTags.h"
#include "CCtokens.h"

/*
TODO: Insert description of token tags here.

Heuristics for finding correct token tag for app:
You will need to know the tokenid of the token you're trying to find the tag/logbook for.
Look for tags that have been made by the token creator for this tokenid specifically.
If the token creator did not create any tags, look for tags that were made by any pubkey.
Choose the tag(s) that have proven ownership of the biggest amount of the token supply.
Prioritize tags that do not have the "only creator can update" flag.
The earliest tag in this list is your "official" token tag.
If there are multiple tags that meet this criteria in the same block / same timestamp, choose the tag with the txid that has the lowest
numerical value when converting the hex to a number.

Token tag creation:
vin.0: normal input
vin.1: tokens
...
vin.n-1: normal input
vout.0: marker to global CC address w/ OP_RETURN EVAL_TOKENTAGS 'c' version srcpub flags maxupdates updateamount
vout.1: tokens
vout.n-2: normal change
vout.n-1: empty op_return

Token tag update:
vin.0: normal input
vin.1: previous global OP_RETURN marker
...
vin.n-1: normal input
vout.0: marker to global CC address w/ OP_RETURN EVAL_TOKENTAGS 'u' version tokentagid srcpub data updateamount
vout.n-2: normal change
vout.n-1: empty op_return

*/

// --- Start of consensus code ---

int64_t IsTokenTagsvout(struct CCcontract_info *cp,const CTransaction& tx, int32_t v, char* destaddr)
{
	char tmpaddr[64];
	if ( tx.vout[v].scriptPubKey.IsPayToCryptoCondition() != 0 )
	{
		if ( Getscriptaddress(tmpaddr,tx.vout[v].scriptPubKey) > 0 && strcmp(tmpaddr,destaddr) == 0 )
			return(tx.vout[v].nValue);
	}
	return(0);
}

// OP_RETURN data encoders and decoders for all Agreements transactions.
CScript EncodeTokenTagCreateOpRet(uint8_t version, CPubKey srcpub, uint8_t flags, int64_t maxupdates, int64_t updateamount)
{
	CScript opret; uint8_t evalcode = EVAL_TOKENTAGS, funcid = 'c';
	opret << OP_RETURN << E_MARSHAL(ss << evalcode << funcid << version << srcpub << flags << maxupdates << updateamount);
	return(opret);
}
uint8_t DecodeTokenTagCreateOpRet(CScript scriptPubKey, uint8_t &version, CPubKey &srcpub, uint8_t &flags, int64_t &maxupdates, int64_t &updateamount)
{
	std::vector<uint8_t> vopret; uint8_t evalcode, funcid;
	GetOpReturnData(scriptPubKey, vopret);
	if(vopret.size() > 2 && E_UNMARSHAL(vopret, ss >> evalcode; ss >> funcid; ss >> version; ss >> srcpub; ss >> flags; ss >> maxupdates; ss >> updateamount) != 0 && evalcode == EVAL_TOKENTAGS)
		return(funcid);
	return(0);
}

CScript EncodeTokenTagUpdateOpRet(uint8_t version, uint256 tokentagid, CPubKey srcpub, std::string data, int64_t updateamount)
{
	CScript opret; uint8_t evalcode = EVAL_TOKENTAGS, funcid = 'u';
	opret << OP_RETURN << E_MARSHAL(ss << evalcode << funcid << version << tokentagid << srcpub << data << updateamount);
	return(opret);
}
uint8_t DecodeTokenTagUpdateOpRet(CScript scriptPubKey, uint8_t &version, uint256 &tokentagid, CPubKey &srcpub, std::string &data, int64_t &updateamount)
{
	std::vector<uint8_t> vopret; uint8_t evalcode, funcid;
	GetOpReturnData(scriptPubKey, vopret);
	if(vopret.size() > 2 && E_UNMARSHAL(vopret, ss >> evalcode; ss >> funcid; ss >> version; ss >> tokentagid; ss >> srcpub; ss >> data; ss >> updateamount) != 0 && evalcode == EVAL_TOKENTAGS)
		return(funcid);
	return(0);
}

// Generic decoder for Token Tags transactions, returns function id.
uint8_t DecodeTokenTagOpRet(const CScript scriptPubKey)
{
	std::vector<uint8_t> vopret;
	CPubKey dummypubkey;
	int64_t dummyint64;
	uint256 dummyuint256;
	std::string dummystring;
	uint8_t evalcode, funcid, *script, dummyuint8;
	GetOpReturnData(scriptPubKey, vopret);
	script = (uint8_t *)vopret.data();
	if(script != NULL && vopret.size() > 2)
	{
		evalcode = script[0];
		if (evalcode != EVAL_TOKENTAGS)
		{
			LOGSTREAM("tokentagscc", CCLOG_DEBUG1, stream << "script[0] " << script[0] << " != EVAL_TOKENTAGS" << std::endl);
			return (uint8_t)0;
		}
		funcid = script[1];
		LOGSTREAM((char *)"tokentagscc", CCLOG_DEBUG2, stream << "DecodeTokenTagOpRet() decoded funcId=" << (char)(funcid ? funcid : ' ') << std::endl);
		switch (funcid)
		{
			case 'c':
				return DecodeTokenTagCreateOpRet(scriptPubKey, dummyuint8, dummypubkey, dummyuint8, dummyint64, dummyint64);
			case 'u':
				return DecodeTokenTagUpdateOpRet(scriptPubKey, dummyuint8, dummyuint256, dummypubkey, dummystring, dummyint64);
			default:
				LOGSTREAM((char *)"tokentagscc", CCLOG_DEBUG1, stream << "DecodeTokenTagOpRet() illegal funcid=" << (int)funcid << std::endl);
				return (uint8_t)0;
		}
	}
	else
		LOGSTREAM("tokentagscc",CCLOG_DEBUG1, stream << "not enough opret.[" << vopret.size() << "]" << std::endl);
	return (uint8_t)0;
}

// Validator for normal inputs in TokenTags transactions.
bool ValidateTokenTagsNormalVins(Eval* eval, const CTransaction& tx, int32_t index)
{
	for (int i=index;i<tx.vin.size();i++)
		if (IsCCInput(tx.vin[i].scriptSig) != 0 )
			return eval->Invalid("vin."+std::to_string(i)+" is normal for tokentags tx!");
	return (true);
}

// Validator for CC inputs found in TokenTags transactions.
bool ValidateTokenTagsCCVin(struct CCcontract_info *cp,Eval* eval,const CTransaction& tx,int32_t index,int32_t prevVout,uint256 prevtxid,char* fromaddr,int64_t amount)
{
	CTransaction prevTx;
	uint256 hashblock;
	int32_t numvouts;
	char tmpaddr[64];

	// Check if a vin is a TokenTags CC vin.
	if ((*cp->ismyvin)(tx.vin[index].scriptSig) == 0)
		return eval->Invalid("vin."+std::to_string(index)+" is TokenTags CC for TokenTags tx!");

	// Verify previous transaction and its op_return.
	else if (myGetTransaction(tx.vin[index].prevout.hash,prevTx,hashblock) == 0)
		return eval->Invalid("vin."+std::to_string(index)+" tx does not exist!");
	else if ((numvouts=prevTx.vout.size()) == 0 || !MyGetCCopretV2(prevTx.vout[0].scriptPubKey, opret) || DecodeTokenTagOpRet(opret) == 0) 
		return eval->Invalid("invalid vin."+std::to_string(index)+" tx OP_RETURN data!");
	
	// If fromaddr != 0, validate prevout dest address.
	else if (fromaddr!=0 && Getscriptaddress(tmpaddr,prevTx.vout[tx.vin[index].prevout.n].scriptPubKey) && strcmp(tmpaddr,fromaddr)!=0)
		return eval->Invalid("invalid vin."+std::to_string(index)+" address!");

	// if amount > 0, validate amount.
	else if (amount>0 && prevTx.vout[tx.vin[index].prevout.n].nValue!=amount)
		return eval->Invalid("vin."+std::to_string(index)+" invalid amount!");
	
	// if prevVout >= 0, validate spent vout number.
	else if (prevVout>=0 && tx.vin[index].prevout.n!=prevVout)
		return eval->Invalid("vin."+std::to_string(index)+" invalid prevout number, expected "+std::to_string(prevVout)+", got "+std::to_string(tx.vin[index].prevout.n)+"!");

	// Validate previous txid.
	else if (prevTx.GetHash()!=prevtxid)
		return eval->Invalid("invalid vin."+std::to_string(index)+" tx, expecting "+prevtxid.GetHex()+", got "+prevTx.GetHash().GetHex()+" !");
	
	return (true);
}

// TODO: all this shit
// Get latest updateamount from previous transaction.
int64_t GetLatestRequiredUpdateAmount(const CTransaction updatetx)
{
    
}

// Get this update's number count, compare it with maxupdates to make sure we don't exceed max allowed amount.
int64_t GetTagUpdateNumber(const CTransaction updatetx)
{

}

// Gets all valid token IDs from the token tag create tx.
// In order for a token vout to be considered valid it must:
// - Have a CC opret with a tokenid and single destination pubkey equal to destpub
// - Be a valid token vout (check with IsTokensvout)
// - Have its nValue be equal to the full supply of the specified token
std::vector<uint256> GetValidTagTokenIds(const CTransaction& createtx, CPubKey destpub)
{
	CScript opret;
	std::vector<uint256> tokenidlist;
	std::vector<CPubKey> voutPubkeys;
    std::vector<vscript_t> oprets;
	uint256 tokenid;
	int32_t numvouts;

	struct CCcontract_info *cpTokens, tokensC;
	cpTokens = CCinit(&tokensC, EVAL_TOKENS);

	numvouts = createtx.vout.size();

	for (int i = 0; i<numvouts; i++)
	{
		if (MyGetCCopretV2(createtx.vout[i].scriptPubKey, opret) &&
		DecodeTokenOpRetV1(opret, tokenid, voutPubkeys, oprets) &&
		voutPubkeys.size() == 1 && voutPubkeys[0] == destpub &&
		IsTokensvout(true, true, cpTokens, NULL, createtx, i, tokenid) &&
		createtx.vout[i].nValue == CCfullsupply(tokenid))
		{
			tokenidlist.push_back(tokenid);
		}
	}

	return tokenidlist;
}

bool ValidateTokenTagCreateTx(struct CCcontract_info *cp,Eval* eval,const CTransaction& createtx)
{
    // TODO: variables
	std::vector<uint256> tokenidlist;

    LOGSTREAM("agreementscc", CCLOG_INFO, stream << "ValidateTokenTagCreateTx: initiated" << std::endl);

	//numvins = createtx.vin.size();
	//numvouts = createtx.vout.size();
	
	CCOpretCheck(eval,createtx,true,true,true);
	ExactAmounts(eval,createtx,ASSETCHAINS_CCZEROTXFEE[EVAL_TOKENTAGS]?0:CC_TXFEE);

	GetCCaddress(cp, globalCCaddress, GetUnspendable(cp, NULL));

	// Check vout0, make sure its embedded opret is correct.
	if (!MyGetCCopretV2(createtx.vout[0].scriptPubKey, opret) || DecodeTokenTagCreateOpRet(opret,version,origsrcpub,
    flags,maxupdates,origupdateamount) != 'c')
        return eval->Invalid("Token tag creation transaction data invalid!");
	
	// Verify that vout0 is a TokenTags vout and that it has the correct value and destination.
	else if (ConstrainVout(createtx.vout[0],1,globalCCaddress,CC_MARKER_VALUE)==0)
		return eval->Invalid("vout.0 is tokentags CC marker vout to global CC address for token tag create transaction!");
    
	// Check vin0, verify that it is a normal input and that it was signed by srcpub.
	else if (IsCCInput(createtx.vin[0].scriptSig) != 0 || TotalPubkeyNormalInputs(tx,srcpub) == 0)
		return eval->Invalid("vin.0 of token tag create transaction must be normal input signed by creator pubkey!");
	
	// Check other vouts, make sure there's at least one Tokens vout, with valid tokenid and full token supply value.
	// TokensValidate should make sure that the corresponding Tokens vin amounts match the value of the vouts.
	tokenlist = GetValidTagTokenIds(createtx, srcpub);
	if (tokenlist.empty())
		return eval->Invalid("no valid token vouts found in token tag create transaction!");

    return true;
}

bool TokenTagsValidate(struct CCcontract_info *cp, Eval* eval, const CTransaction &tx, uint32_t nIn)
{
	return eval->Invalid("not supported yet");
    // TODO: variables

	// Check boundaries, and verify that input/output amounts are exact.
	numvins = tx.vin.size();
	numvouts = tx.vout.size();
	if (numvouts < 1)
		return eval->Invalid("No vouts!");

	CCOpretCheck(eval,tx,true,true,true);
	ExactAmounts(eval,tx,ASSETCHAINS_CCZEROTXFEE[EVAL_TOKENTAGS]?0:CC_TXFEE);

	// Check the op_return of the transaction and fetch its function id.
	if (!MyGetCCopretV2(tx.vout[0].scriptPubKey, opret) || (funcid = DecodeTokenTagOpRet(opret)) != 0)
	{
		GetCCaddress(cp, globalCCaddress, GetUnspendable(cp, NULL));

		switch (funcid)
		{
			case 'c':
				// Token tag creation:
                // vin.0: normal input
                // vin.1: tokens
                // ...
                // vin.n-1: normal input
                // vout.0: marker to global CC address w/ OP_RETURN EVAL_TOKENTAGS 'c' version srcpub flags maxupdates updateamount
                // vout.1: tokens
                // vout.n-2: normal change
				// vout.n-1: empty op_return

				// Create transactions shouldn't trigger validation directly as they shouldn't contain tokentags CC inputs. 
				return eval->Invalid("unexpected TokenTagsValidate for 'c' type transaction!");

            case 'u':
				// Token tag update:
                // vin.0: normal input
                // vin.1: previous global OP_RETURN marker
                // ...
                // vin.n-1: normal input
                // vout.0: marker to global CC address w/ OP_RETURN EVAL_TOKENTAGS 'u' version tokentagid srcpub data updateamount
                // vout.n-2: normal change
				// vout.n-1: empty op_return

                // Get the data from the transaction's op_return.
                DecodeTokenTagUpdateOpRet(opret,version,tokentagid,srcpub,data,updateamount);
				
                // Check appended data length (should be <= 128 chars).
				if (data.size() > 128)
					return eval->Invalid("data string over 128 chars!");
                
                // Get the tokentagid transaction.
				else if (myGetTransaction(tokentagid,tokentagtx,hashBlock) == 0 || tokentagtx.vout.size() == 0 ||
				!MyGetCCopretV2(tokentagtx.vout[0].scriptPubKey,opret) || DecodeTokenTagOpRet(opret) != 'c')
					return eval->Invalid("Original token tag create transaction not found!");

				// Validate the tokentagid transaction.
                else if (ValidateTokenTagCreateTx(cp,eval,tokentagtx) == 0)
					return (false);
                
                // Get the data from the tokentagid transaction's op_return.
				DecodeTokenTagCreateOpRet(opret,version,origsrcpub,flags,maxupdates,origupdateamount);

                // If the TTF_CREATORONLY flag is set, make sure the update is signed by original srcpub.
                if (flags & TTF_CREATORONLY && srcpub != origsrcpub)
					return eval->Invalid("Signing pubkey of tag update transaction is not the tag creator pubkey!");
				
                // If the TTF_CONSTREQS flag is set, make sure the required token amounts for updates are kept the same.
                if (flags & TTF_CONSTREQS && updateamount != origupdateamount)
					return eval->Invalid("New required token amounts for updates are not the same as original requirements!");
                
                // TODO: Get latest updateamount from previous transaction.

                // TODO: Get this update's number count, compare it with maxupdates to make sure we don't exceed max allowed amount.

                // Get the tokenids from the create tx.
				tokenlist = GetValidTagTokenIds(createtx, origsrcpub);

                // Check token balance of tokenid from srcpub, at this blockheight. Compare with latest updateamount.
				for (auto tokenid : tokenlist)
				{
					//TODO fix: if (GetTokenBalance(srcpub, tokenid, false) < updateamount)
						return eval->Invalid("Creator pubkey of token tag update doesn't own enough tokens!");
				}

				// Check vout boundaries for tag update transaction.
                if (numvouts > 2)
					return eval->Invalid("Too many vouts for 's' type transaction!");
                
                // Verify that vin.0 was signed by srcpub.
				else if (IsCCInput(tx.vin[0].scriptSig) != 0 || TotalPubkeyNormalInputs(tx,srcpub) == 0)
					return eval->Invalid("vin.0 must be normal input signed by transaction creator pubkey!");

				// Verify that vin.1 was signed correctly.
				else if (ValidateTokenTagsCCVin(cp,eval,tx,1,0,prevupdatetxid,srcCCaddress,0) == 0)
					return (false);

				// Token tag updates shouldn't have any additional CC inputs.
				else if (ValidateTokenTagsNormalVins(eval,tx,2) == 0)
					return (false);

                // vout.0 must be global marker to CC address.
                else if (ConstrainVout(tx.vout[0], 1, globalCCaddress, CC_MARKER_VALUE) == 0)
					return eval->Invalid("vout.0 must be CC marker to TokenTags global CC address!");

				break;

			default:
				fprintf(stderr,"unexpected tokentags funcid (%c)\n",funcid);
				return eval->Invalid("Unexpected TokenTags function id!");
		}
	}
	else
		return eval->Invalid("Invalid TokenTags function id and/or data!");

	LOGSTREAM("tokentagscc", CCLOG_INFO, stream << "TokenTags transaction validated" << std::endl);
	return (true);
}

// --- End of consensus code ---

// --- Helper functions for RPC implementations ---

// --- RPC implementations for transaction creation ---

// tokentagcreate tokenids [updateamount][flags][maxupdates]
// tokentagupdate tokentagid "data" [updateamount]
// tokentagclose tokentagid "data"

// --- RPC implementations for transaction analysis ---

// tokentaglist [tokenid][pubkey]
// tokentaginfo tokentagid
// tokentagsamples tokentagid [samplenum][reverse]

// --- Useful misc functions for additional token transaction analysis ---

// internal function that looks for voutPubkeys in token tx oprets
// used by TokenOwners
void GetTokenOwnerPubkeys(const CTransaction tx, struct CCcontract_info *cp, uint256 tokenid, std::vector<CPubKey> &OwnerList)
{
    // TODO: add "max depth" variable to avoid stack overflows from recursive calls

    // Examine each vout in the tx (except last vout)
    for (int64_t n = 0; n <= tx.vout.size() - 1; n++)
    {
        CScript opret;
        uint256 tokenIdOpret, spendingtxid, hashBlock;
        CTransaction spendingtx;
        std::vector<vscript_t>  oprets;
        std::vector<CPubKey> voutPubkeys;
        int32_t vini, height;

        // We ignore every vout that's not a tokens vout, so we check for that
        if (IsTokensvout(true, true, cp, NULL, tx, n, tokenid))
        {
            // Get the opret from either vout.n or tx.vout.back() scriptPubkey
            if (!MyGetCCopretV2(tx.vout[n].scriptPubKey, opret))
                opret = tx.vout.back().scriptPubKey;
            uint8_t funcId = DecodeTokenOpRetV1(opret, tokenIdOpret, voutPubkeys, oprets);

            // Include only pubkeys from voutPubkeys arrays with 1 element.
            // If voutPubkeys size is >= 2 then the vout was probably sent to a CC 1of2 address, which
            // might have no true "owner" and is therefore outside the scope of this function
            if (voutPubkeys.size() == 1)
            {
                // Check if found pubkey is already in the list, if not, add it
                std::vector<CPubKey>::iterator it = std::find(OwnerList.begin(), OwnerList.end(), voutPubkeys[0]);
                if (it == OwnerList.end())
                    OwnerList.push_back(voutPubkeys[0]);
            }

            // Check if this vout was spent, and if it was, find the tx that spent it
            if (CCgetspenttxid(spendingtxid, vini, height, tx.GetHash(), n) == 0 &&
            myGetTransaction(spendingtxid, spendingtx, hashBlock))
            {
                // Same procedure for the spending tx, until no more are found
                GetTokenOwnerPubkeys(spendingtx, cp, tokenid, OwnerList);
            }
        }
    }

    return;
}

UniValue TokenOwners(uint256 tokenid, int64_t minbalance)
{
    // TODO: add option to retrieve addresses instead, including 1of2 addresses
	UniValue result(UniValue::VARR); 
    CTransaction tokenbaseTx; 
    uint256 hashBlock;
	uint8_t funcid;
    std::vector<uint8_t> origpubkey;
    std::string name, description; 
    std::vector<CPubKey> OwnerList;
    char str[67];

    struct CCcontract_info *cpTokens, tokensCCinfo;
    cpTokens = CCinit(&tokensCCinfo, EVAL_TOKENS);

    // Get token create tx
    if (!myGetTransaction(tokenid, tokenbaseTx, hashBlock) || (KOMODO_NSPV_FULLNODE && hashBlock.IsNull()))
    {
        LOGSTREAMFN(cctokens_log, CCLOG_INFO, stream << "cant find tokenid" << std::endl);
        return(result);
    }

    // Checking if passed tokenid is a token creation txid
    funcid = DecodeTokenCreateOpRetV1(tokenbaseTx.vout.back().scriptPubKey, origpubkey, name, description);
	if (tokenbaseTx.vout.size() > 0 && !IsTokenCreateFuncid(funcid))
	{
        LOGSTREAMFN(cctokens_log, CCLOG_INFO, stream << "passed tokenid isnt token creation txid" << std::endl);
        return(result);
    }

    // Get a full list of owner pubkeys using a recursive looping function
    GetTokenOwnerPubkeys(tokenbaseTx, cpTokens, tokenid, OwnerList);
    // TODO: maybe remove/skip known CC global pubkeys from the list?

    // Add pubkeys to result array
    for (auto pk : OwnerList)
        if (minbalance == 0 || GetTokenBalance(pk, tokenid, false) >= minbalance)
            result.push_back(pubkey33_str(str,(uint8_t *)&pk));

	return result;
}

UniValue TokenInventory(const CPubKey pk, int64_t minbalance)
{
    // TODO: add option to specify an address instead of a pubkey
	UniValue result(UniValue::VARR); 
    char tokenaddr[KOMODO_ADDRESS_BUFSIZE];
    std::vector<std::pair<CAddressIndexKey, CAmount> > addressIndex;
    std::vector<uint256> TokenList;

    struct CCcontract_info *cpTokens, tokensCCinfo;
    cpTokens = CCinit(&tokensCCinfo, EVAL_TOKENS);

    // Get token CC address of specified pubkey
    GetTokensCCaddress(cpTokens, tokenaddr, pk);

    // Get all CC outputs sent to this address
    SetCCtxids(addressIndex, tokenaddr, true);
    
    for (std::vector<std::pair<CAddressIndexKey, CAmount> >::const_iterator it = addressIndex.begin(); it != addressIndex.end(); it++) 
    {
        std::vector<vscript_t>  oprets;
        std::vector<CPubKey> voutPubkeys;
        uint256 tokenIdInOpret, hashBlock;
        CTransaction vintx;
        CScript opret;

        // Find the tx that the CC output originates from
        if (myGetTransaction(it->first.txhash, vintx, hashBlock))
        {
            int32_t n = (int32_t)it->first.index;

            // skip markers
            if (IsTokenMarkerVout(vintx.vout[n]))
                continue;

            // Get the opret from either vout.n or tx.vout.back() scriptPubkey
            if (!MyGetCCopretV2(vintx.vout[n].scriptPubKey, opret))
                opret = vintx.vout.back().scriptPubKey;
            uint8_t funcid = DecodeTokenOpRetV1(opret, tokenIdInOpret, voutPubkeys, oprets);

            // If the vout is from a token creation tx, the tokenid will be hash of vintx
            if (IsTokenCreateFuncid(funcid))
                tokenIdInOpret = vintx.GetHash();
            
            // If the vout is not from a token creation tx, check if it is a token vout
            if (IsTokenCreateFuncid(funcid) || IsTokensvout(true, true, cpTokens, NULL, vintx, n, tokenIdInOpret))
            {
                // Check if found tokenid is already in the list, if not, add it
                std::vector<uint256>::iterator it2 = std::find(TokenList.begin(), TokenList.end(), tokenIdInOpret);
                if (it2 == TokenList.end())
                    TokenList.push_back(tokenIdInOpret);
            }
        }
    }
    
    // Add token ids to result array
    for (auto tokenid : TokenList)
        if (minbalance == 0 || GetTokenBalance(pk, tokenid, false) >= minbalance)
            result.push_back(tokenid.GetHex());

	return result;
}
