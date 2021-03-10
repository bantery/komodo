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
#include <stdint.h>
#include <string.h>
#include <numeric>
#include "univalue.h"
#include "amount.h"
#include "rpc/server.h"
#include "rpc/protocol.h"

#include "../wallet/crypter.h"
#include "../wallet/rpcwallet.h"

#include "sync_ext.h"

#include "../cc/CCinclude.h"
#include "../cc/CCtokens.h"
#include "../cc/CCTokenTags.h"

using namespace std;

extern void Lock2NSPV(const CPubKey &pk);
extern void Unlock2NSPV(const CPubKey &pk);

UniValue tokentagcreate(const UniValue& params, bool fHelp, const CPubKey& mypk)
{
    UniValue result(UniValue::VOBJ), tokens(UniValue::VARR);
    std::string hex;

	uint8_t flags;
    uint256 tokenid;
	int64_t maxupdates;
    std::vector<uint256> tokenids;
    std::vector<CAmount> updateamounts;

    if (fHelp || params.size() < 1 || params.size() > 3)
        throw runtime_error(
            "tokentagcreate [{\"tokenid\":... ,\"updateamount\":...},...] ( flags ) ( maxupdates )\n"
            );
    if (ensure_CCrequirements(EVAL_TOKENTAGS) < 0 || ensure_CCrequirements(EVAL_TOKENS) < 0)
        throw runtime_error(CC_REQUIREMENTS_MSG);
    
    CCerror.clear();

    //Lock2NSPV(mypk); // nSPV for tokens is unsupported at the time of writing - Dan

    if (!EnsureWalletIsAvailable(false))
        throw runtime_error("wallet is required");
    LOCK2(cs_main, pwalletMain->cs_wallet);

    tokens = params[0].get_array();
    if (tokens.size() == 0)
        return MakeResultError("Invalid parameter, tokens array is empty."); 

    for (const UniValue& o : tokens.getValues())
    {
        if (!o.isObject())
            return MakeResultError("Invalid parameter, expected object."); 
        
        // sanity check, report error if unknown key-value pairs
        for (const string& name_ : o.getKeys())
        {
            std::string s = name_;
            if (s != "tokenid" && s != "updateamount")
                return MakeResultError(string("Invalid parameter, unknown key: ")+s); 
        }

        tokenid = Parseuint256((char *)find_value(o,"tokenid").get_str().c_str());
        if (tokenid == zeroid)
            return MakeResultError("Invalid parameter, tokenid in object invalid or null"); 

        for (const auto &entry : tokenids) 
        {
            if (entry == tokenid)
                return MakeResultError(string("Invalid parameter, duplicated tokenid: ")+tokenid.GetHex());
        }
        tokenids.push_back(tokenid);
        
        UniValue av = find_value(o, "updateamount");
        CAmount nAmount = AmountFromValue(av);
        if (nAmount < 0)
            return MakeResultError("Invalid parameter, updateamount must be positive"); 
        
        updateamounts.push_back(nAmount);
    }

    if (tokenids.size() != updateamounts.size())
        return MakeResultError("Invalid parameter, mismatched amount of specified tokenids vs updateamounts"); 

    flags = 0;
    if (params.size() >= 2)
        flags = atoll(params[1].get_str().c_str());
    
    maxupdates = 0;
    if (params.size() == 3)
    {
		maxupdates = atoll(params[2].get_str().c_str());
        if (maxupdates < -1)
            return MakeResultError("Invalid maxupdates, must be -1, 0 or any positive number"); 
    }

    //Unlock2NSPV(mypk);

    UniValue sigData = TokenTagCreate(mypk,0,tokenids,updateamounts,flags,maxupdates);
    RETURN_IF_ERROR(CCerror);
    if (ResultHasTx(sigData) > 0)
        result = sigData;
    else
        result = MakeResultError("Could not create token tag: " + ResultGetError(sigData) );
    return(result);
}

UniValue tokentagupdate(const UniValue& params, bool fHelp, const CPubKey& mypk)
{
    UniValue result(UniValue::VOBJ);

	uint256 tokentagid;
	std::string data = "";
    std::vector<CAmount> updateamounts;

    if (fHelp || params.size() < 2 || params.size() > 3)
        throw runtime_error(
            "tokentagupdate tokentagid \"data\" [updateamount]\n"
            );
    if (ensure_CCrequirements(EVAL_TOKENTAGS) < 0 || ensure_CCrequirements(EVAL_TOKENS) < 0)
        throw runtime_error(CC_REQUIREMENTS_MSG);
    
    Lock2NSPV(mypk);

    tokentagid = Parseuint256((char *)params[0].get_str().c_str());
	if (tokentagid == zeroid)
    {
		Unlock2NSPV(mypk);
        throw runtime_error("Token tag id invalid\n");
    }
    
    data = params[1].get_str();
    if (data.empty() || data.size() > 128)
    {
        Unlock2NSPV(mypk);
        throw runtime_error("Data string must not be empty and be up to 128 characters\n");
    }

	if (params.size() == 3)
    {
        //TODO: how to pass an array of CAmount here?
    }

    result = TokenTagUpdate(mypk,0,tokentagid,data,updateamounts);
    if (result[JSON_HEXTX].getValStr().size() > 0)
        result.push_back(Pair("result", "success"));
    Unlock2NSPV(mypk);
    return(result);
}

UniValue tokentagclose(const UniValue& params, bool fHelp, const CPubKey& mypk)
{
    UniValue result(UniValue::VOBJ);

	uint256 tokentagid;
	std::string data = "";

    if (fHelp || params.size() != 2)
        throw runtime_error(
            "tokentagclose tokentagid \"data\"\n"
            );
    if (ensure_CCrequirements(EVAL_TOKENTAGS) < 0 || ensure_CCrequirements(EVAL_TOKENS) < 0)
        throw runtime_error(CC_REQUIREMENTS_MSG);
    
    Lock2NSPV(mypk);

    tokentagid = Parseuint256((char *)params[0].get_str().c_str());
	if (tokentagid == zeroid)
    {
		Unlock2NSPV(mypk);
        throw runtime_error("Token tag id invalid\n");
    }
    
    data = params[1].get_str();
    if (data.empty() || data.size() > 128)
    {
        Unlock2NSPV(mypk);
        throw runtime_error("Data string must not be empty and be up to 128 characters\n");
    }

    result = TokenTagClose(mypk,0,tokentagid,data);
    if (result[JSON_HEXTX].getValStr().size() > 0)
        result.push_back(Pair("result", "success"));
    Unlock2NSPV(mypk);
    return(result);
}

// Additional RPCs for token transaction analysis

UniValue tokenowners(const UniValue& params, bool fHelp, const CPubKey& mypk)
{
    uint256 tokenid;
    int64_t minbalance = 1;
    if ( fHelp || params.size() < 1 || params.size() > 2 )
        throw runtime_error("tokenowners tokenid [minbalance]\n");
    if ( ensure_CCrequirements(EVAL_TOKENS) < 0 )
        throw runtime_error(CC_REQUIREMENTS_MSG);
    tokenid = Parseuint256((char *)params[0].get_str().c_str());
    bool bShowAll = false;
    if (params.size() == 2)
        minbalance = atoll(params[1].get_str().c_str()); 
    return(TokenOwners(tokenid,minbalance));
}

UniValue tokeninventory(const UniValue& params, bool fHelp, const CPubKey& mypk)
{
    std::vector<unsigned char> vpubkey;
    int64_t minbalance = 1;
    if ( fHelp || params.size() > 2 )
        throw runtime_error("tokeninventory [minbalance][pubkey]\n");
    if ( ensure_CCrequirements(EVAL_TOKENS) < 0 )
        throw runtime_error(CC_REQUIREMENTS_MSG);
    if (params.size() >= 1)
        minbalance = atoll(params[0].get_str().c_str()); 
    if (params.size() == 2)
        vpubkey = ParseHex(params[1].get_str().c_str());
    else 
		vpubkey = Mypubkey();
    return(TokenInventory(pubkey2pk(vpubkey),minbalance));
}

UniValue tokentagaddress(const UniValue& params, bool fHelp, const CPubKey& mypk)
{
    struct CCcontract_info *cp,C; std::vector<unsigned char> pubkey;
    cp = CCinit(&C,EVAL_TOKENTAGS);
    if ( fHelp || params.size() > 1 )
        throw runtime_error("tokentagaddress [pubkey]\n");
    if ( ensure_CCrequirements(0) < 0 )
        throw runtime_error(CC_REQUIREMENTS_MSG);
    if ( params.size() == 1 )
        pubkey = ParseHex(params[0].get_str().c_str());
    return(CCaddress(cp,(char *)"TokenTags",pubkey));
}

static const CRPCCommand commands[] =
{ //  category              name                actor (function)        okSafeMode
  //  -------------- ------------------------  -----------------------  ----------
    // extended tokens
	{ "tokens",    "tokenowners",     &tokenowners,     true },
    { "tokens",    "tokeninventory",  &tokeninventory,  true },
    // token tags
	{ "tokentags", "tokentagaddress", &tokentagaddress, true },
    { "tokentags", "tokentagcreate",  &tokentagcreate,  true },
    { "tokentags", "tokentagupdate",  &tokentagupdate,  true },
    { "tokentags", "tokentagclose",   &tokentagclose,   true },
	//{ "tokentags",  "tokentaginfo",    &tokentaginfo,	true },
};

void RegisterTokenTagsRPCCommands(CRPCTable &tableRPC)
{
    for (unsigned int vcidx = 0; vcidx < ARRAYLEN(commands); vcidx++)
        tableRPC.appendCommand(commands[vcidx].name, &commands[vcidx]);
}
