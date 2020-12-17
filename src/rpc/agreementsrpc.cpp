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
#include "../cc/CCagreements.h"

using namespace std;

extern void Lock2NSPV(const CPubKey &pk);
extern void Unlock2NSPV(const CPubKey &pk);

UniValue agreementaddress(const UniValue& params, bool fHelp, const CPubKey& mypk)
{
    struct CCcontract_info *cp,C; std::vector<unsigned char> pubkey;
    cp = CCinit(&C,EVAL_AGREEMENTS);
    if ( fHelp || params.size() > 1 )
        throw runtime_error("agreementaddress [pubkey]\n");
    if ( ensure_CCrequirements(0) < 0 )
        throw runtime_error(CC_REQUIREMENTS_MSG);
    if ( params.size() == 1 )
        pubkey = ParseHex(params[0].get_str().c_str());
    return(CCaddress(cp,(char *)"Agreements",pubkey));
}

UniValue agreementcreate(const UniValue& params, bool fHelp, const CPubKey& mypk)
{
    UniValue result(UniValue::VOBJ);

    CPubKey destpub,arbitratorpub;
	uint256 agreementhash,refagreementtxid;
	std::string agreementname;
	int64_t payment,disputefee,deposit;

    if (fHelp || params.size() < 4 || params.size() > 8)
        throw runtime_error(
            "agreementcreate destpub agreementname agreementhash deposit [arbitratorpub][disputefee][refagreementtxid][payment]\n"
            );
    if (ensure_CCrequirements(EVAL_AGREEMENTS) < 0)
        throw runtime_error(CC_REQUIREMENTS_MSG);
    
    Lock2NSPV(mypk);
	
    destpub = pubkey2pk(ParseHex(params[0].get_str().c_str()));

    agreementname = params[1].get_str();
    if (agreementname.size() == 0 || agreementname.size() > 64)
    {
		Unlock2NSPV(mypk);
        throw runtime_error("Agreement name must not be empty and up to 64 characters\n");
    }

    agreementhash = Parseuint256((char *)params[2].get_str().c_str());
	if (agreementhash == zeroid)
    {
		Unlock2NSPV(mypk);
        throw runtime_error("Agreement hash empty or invalid\n");
    }

    deposit = atoll(params[3].get_str().c_str()) * COIN;
    if (deposit >= 0)
        deposit = AmountFromValue(params[3]);
    else
    {
        Unlock2NSPV(mypk);
        throw runtime_error("Required deposit too low\n");
    }
    
    if (params.size() >= 5)
        arbitratorpub = pubkey2pk(ParseHex(params[4].get_str().c_str()));

    disputefee = 0;
	if (params.size() >= 6)
    {
        if (arbitratorpub.IsFullyValid())
        {
            disputefee = atoll(params[5].get_str().c_str()) * COIN;
            if (disputefee == 0 || disputefee >= 10000)
                disputefee = AmountFromValue(params[5]);
            else
            {
                Unlock2NSPV(mypk);
                throw runtime_error("Dispute fee too low\n");
            }
        }
    }

    if (params.size() >= 7)
        refagreementtxid = Parseuint256((char *)params[6].get_str().c_str());

	payment = 0;
	if (params.size() == 8)
    {
        payment = atoll(params[7].get_str().c_str()) * COIN;
        if (payment == 0 || payment >= 10000)
            payment = AmountFromValue(params[7]);
        else
        {
            Unlock2NSPV(mypk);
            throw runtime_error("Required prepayment too low\n");
        }
    }
	
	result = AgreementCreate(mypk,0,destpub,agreementname,agreementhash,deposit,arbitratorpub,disputefee,refagreementtxid,payment);
    if (result[JSON_HEXTX].getValStr().size() > 0)
        result.push_back(Pair("result", "success"));
    Unlock2NSPV(mypk);
    return(result);
}

UniValue agreementupdate(const UniValue& params, bool fHelp, const CPubKey& mypk)
{
    UniValue result(UniValue::VOBJ);

	uint256 agreementhash,agreementtxid;
	std::string agreementname = "";
	int64_t payment;

    if (fHelp || params.size() < 2 || params.size() > 4)
        throw runtime_error(
            "agreementupdate agreementtxid agreementhash [agreementname][payment]\n"
            );
    if (ensure_CCrequirements(EVAL_AGREEMENTS) < 0)
        throw runtime_error(CC_REQUIREMENTS_MSG);
    
    Lock2NSPV(mypk);

    agreementtxid = Parseuint256((char *)params[0].get_str().c_str());
	if (agreementtxid == zeroid)   {
		Unlock2NSPV(mypk);
        throw runtime_error("Agreement transaction id invalid\n");
    }

    agreementhash = Parseuint256((char *)params[1].get_str().c_str());
	if (agreementhash == zeroid)
    {
		Unlock2NSPV(mypk);
        throw runtime_error("Agreement hash empty or invalid\n");
    }

    if (params.size() >= 3)
    {
        agreementname = params[2].get_str();
        if (agreementname.size() > 64)
        {
            Unlock2NSPV(mypk);
            throw runtime_error("Agreement name must be up to 64 characters\n");
        }
    }

	payment = 0;
	if (params.size() == 4)
    {
        payment = atoll(params[3].get_str().c_str()) * COIN;
        if (payment == 0 || payment >= 10000)
            payment = AmountFromValue(params[3]);
        else
        {
            Unlock2NSPV(mypk);
            throw runtime_error("Required payment too low\n");
        }
    }

	result = AgreementUpdate(mypk,0,agreementtxid,agreementhash,agreementname,payment);
    if (result[JSON_HEXTX].getValStr().size() > 0)
        result.push_back(Pair("result", "success"));
    Unlock2NSPV(mypk);
    return(result);
}

UniValue agreementclose(const UniValue& params, bool fHelp, const CPubKey& mypk)
{
    UniValue result(UniValue::VOBJ);

	uint256 agreementhash,agreementtxid;
	std::string agreementname = "";
	int64_t payment, depositcut;

    if (fHelp || params.size() < 3 || params.size() > 5)
        throw runtime_error(
            "agreementclose agreementtxid agreementhash depositcut [agreementname][payment]\n"
            );
    if (ensure_CCrequirements(EVAL_AGREEMENTS) < 0)
        throw runtime_error(CC_REQUIREMENTS_MSG);
    
    Lock2NSPV(mypk);

    agreementtxid = Parseuint256((char *)params[0].get_str().c_str());
	if (agreementtxid == zeroid)   {
		Unlock2NSPV(mypk);
        throw runtime_error("Agreement transaction id invalid\n");
    }

    agreementhash = Parseuint256((char *)params[1].get_str().c_str());
	if (agreementhash == zeroid)
    {
		Unlock2NSPV(mypk);
        throw runtime_error("Agreement hash empty or invalid\n");
    }

    depositcut = atoll(params[2].get_str().c_str()) * COIN;
    if (depositcut >= 0)
        depositcut = AmountFromValue(params[2]);
    else
    {
        Unlock2NSPV(mypk);
        throw runtime_error("Required deposit cut can't be negative\n");
    }

    if (params.size() >= 4)
    {
        agreementname = params[3].get_str();
        if (agreementname.size() > 64)
        {
            Unlock2NSPV(mypk);
            throw runtime_error("Agreement name must be up to 64 characters\n");
        }
    }

	payment = 0;
	if (params.size() == 5)
    {
        payment = atoll(params[4].get_str().c_str()) * COIN;
        if (payment == 0 || payment >= 10000)
            payment = AmountFromValue(params[4]);
        else
        {
            Unlock2NSPV(mypk);
            throw runtime_error("Required payment too low\n");
        }
    }

    result = AgreementClose(mypk,0,agreementtxid,agreementhash,depositcut,agreementname,payment);
    if (result[JSON_HEXTX].getValStr().size() > 0)
        result.push_back(Pair("result", "success"));
    Unlock2NSPV(mypk);
    return(result);
}

UniValue agreementstopproposal(const UniValue& params, bool fHelp, const CPubKey& mypk)
{
    UniValue result(UniValue::VOBJ);

	uint256 proposaltxid;
	std::string cancelinfo = "";

    if (fHelp || params.size() < 1 || params.size() > 2)
        throw runtime_error(
            "agreementstopproposal proposaltxid [cancelinfo]\n"
            );
    if (ensure_CCrequirements(EVAL_AGREEMENTS) < 0)
        throw runtime_error(CC_REQUIREMENTS_MSG);
    
    Lock2NSPV(mypk);

    proposaltxid = Parseuint256((char *)params[0].get_str().c_str());
	if (proposaltxid == zeroid)   {
		Unlock2NSPV(mypk);
        throw runtime_error("Proposal transaction id invalid\n");
    }

    if (params.size() == 2)
    {
        cancelinfo = params[1].get_str();
        if (cancelinfo.size() > 256)
        {
            Unlock2NSPV(mypk);
            throw runtime_error("Cancel info must be up to 256 characters\n");
        }
    }

    result = AgreementStopProposal(mypk,0,proposaltxid,cancelinfo);
    if (result[JSON_HEXTX].getValStr().size() > 0)
        result.push_back(Pair("result", "success"));
    Unlock2NSPV(mypk);
    return(result);
}

UniValue agreementaccept(const UniValue& params, bool fHelp, const CPubKey& mypk)
{
    UniValue result(UniValue::VOBJ);

	uint256 proposaltxid;

    if (fHelp || params.size() != 1)
        throw runtime_error(
            "agreementaccept proposaltxid\n"
            );
    if (ensure_CCrequirements(EVAL_AGREEMENTS) < 0)
        throw runtime_error(CC_REQUIREMENTS_MSG);
    
    Lock2NSPV(mypk);

    proposaltxid = Parseuint256((char *)params[0].get_str().c_str());
	if (proposaltxid == zeroid)   {
		Unlock2NSPV(mypk);
        throw runtime_error("Proposal transaction id invalid\n");
    }

    result = AgreementAccept(mypk,0,proposaltxid);
    if (result[JSON_HEXTX].getValStr().size() > 0)
        result.push_back(Pair("result", "success"));
    Unlock2NSPV(mypk);
    return(result);
}

UniValue agreementdispute(const UniValue& params, bool fHelp, const CPubKey& mypk)
{
    UniValue result(UniValue::VOBJ);

	uint256 agreementtxid;
	std::string typestr,disputeinfo = "";
    bool bFinalDispute;

    if (fHelp || params.size() < 1 || params.size() > 3)
        throw runtime_error(
            "agreementdispute agreementtxid [disputeinfo][bFinalDispute]\n"
            );
    if (ensure_CCrequirements(EVAL_AGREEMENTS) < 0)
        throw runtime_error(CC_REQUIREMENTS_MSG);
    
    Lock2NSPV(mypk);

    agreementtxid = Parseuint256((char *)params[0].get_str().c_str());
	if (agreementtxid == zeroid)   {
		Unlock2NSPV(mypk);
        throw runtime_error("Agreement transaction id invalid\n");
    }

    if (params.size() >= 2)
    {
        disputeinfo = params[1].get_str();
        if (disputeinfo.size() > 256)
        {
            Unlock2NSPV(mypk);
            throw runtime_error("Dispute info must be up to 256 characters\n");
        }
    }

	bFinalDispute = false;
	if (params.size() == 3)
    {
        typestr = params[2].get_str(); // NOTE: is there a better way to parse a bool from the param array?
        if (STR_TOLOWER(typestr) == "1" || STR_TOLOWER(typestr) == "true")
            bFinalDispute = true;
    }

    result = AgreementDispute(mypk,0,agreementtxid,disputeinfo,bFinalDispute);
    if (result[JSON_HEXTX].getValStr().size() > 0)
        result.push_back(Pair("result", "success"));
    Unlock2NSPV(mypk);
    return(result);
}

UniValue agreementresolve(const UniValue& params, bool fHelp, const CPubKey& mypk)
{
    UniValue result(UniValue::VOBJ);

	uint256 disputetxid;
	std::string resolutioninfo = "";
    int64_t depositcut;

    if (fHelp || params.size() < 2 || params.size() > 3)
        throw runtime_error(
            "agreementresolve disputetxid depositcut [resolutioninfo]\n"
            );
    if (ensure_CCrequirements(EVAL_AGREEMENTS) < 0)
        throw runtime_error(CC_REQUIREMENTS_MSG);
    
    Lock2NSPV(mypk);

    disputetxid = Parseuint256((char *)params[0].get_str().c_str());
	if (disputetxid == zeroid)   {
		Unlock2NSPV(mypk);
        throw runtime_error("Dispute transaction id invalid\n");
    }
    
    depositcut = atoll(params[1].get_str().c_str()) * COIN;
    if (depositcut >= 0)
        depositcut = AmountFromValue(params[1]);

	if (params.size() == 3)
    {
        resolutioninfo = params[2].get_str();
        if (resolutioninfo.size() > 256)
        {
            Unlock2NSPV(mypk);
            throw runtime_error("Dispute resolution info must be up to 256 characters\n");
        }
    }

    result = AgreementResolve(mypk,0,disputetxid,depositcut,resolutioninfo);
    if (result[JSON_HEXTX].getValStr().size() > 0)
        result.push_back(Pair("result", "success"));
    Unlock2NSPV(mypk);
    return(result);
}

UniValue agreementunlock(const UniValue& params, bool fHelp, const CPubKey& mypk)
{
    UniValue result(UniValue::VOBJ);

	uint256 agreementtxid,unlocktxid;

    if (fHelp || params.size() != 2)
        throw runtime_error(
            "agreementunlock agreementtxid unlocktxid\n"
            );
    if (ensure_CCrequirements(EVAL_AGREEMENTS) < 0)
        throw runtime_error(CC_REQUIREMENTS_MSG);
    
    Lock2NSPV(mypk);

    agreementtxid = Parseuint256((char *)params[0].get_str().c_str());
	if (agreementtxid == zeroid)   {
		Unlock2NSPV(mypk);
        throw runtime_error("Agreement transaction id invalid\n");
    }

    unlocktxid = Parseuint256((char *)params[0].get_str().c_str());
	if (unlocktxid == zeroid)   {
		Unlock2NSPV(mypk);
        throw runtime_error("Unlocking transaction id invalid\n");
    }

    result = AgreementUnlock(mypk,0,agreementtxid,unlocktxid);
    if (result[JSON_HEXTX].getValStr().size() > 0)
        result.push_back(Pair("result", "success"));
    Unlock2NSPV(mypk);
    return(result);
}

UniValue agreementinfo(const UniValue& params, bool fHelp, const CPubKey& mypk)
{
    uint256 txid;
    if ( fHelp || params.size() != 1 )
        throw runtime_error(
            "agreementinfo txid\n"
            "\nReturns info about any Agreements CC transaction.\n"
            + HelpRequiringPassphrase() +
            "\nArguments:\n"
            "1. txid (uint256, required) ID of an Agreements CC transaction.\n"
            "\nResult:\n"
            "\"result\"  (string) Info about the transaction.\n"
            "\nExamples:\n"
            + HelpExampleCli("agreementinfo", "56b9bae388690d42fb13c7431d935acbda209bdafa239531549ab4de4b20802a")
            + HelpExampleRpc("agreementinfo", "56b9bae388690d42fb13c7431d935acbda209bdafa239531549ab4de4b20802a")
        );
    if ( ensure_CCrequirements(EVAL_AGREEMENTS) < 0 )
        throw runtime_error(CC_REQUIREMENTS_MSG);
    txid = Parseuint256((char *)params[0].get_str().c_str());
    return(AgreementInfo(txid));
}

// agreementeventlog (agreementupdatelog, with filter!)

UniValue agreementreferences(const UniValue& params, bool fHelp, const CPubKey& mypk)
{
    uint256 agreementtxid;

    if ( fHelp || params.size() != 1 )
        throw runtime_error(
            "agreementreferences agreementtxid\n"
            );
    if ( ensure_CCrequirements(EVAL_AGREEMENTS) < 0 )
        throw runtime_error(CC_REQUIREMENTS_MSG);
    
    agreementtxid = Parseuint256((char *)params[0].get_str().c_str());
    if (agreementtxid == zeroid)
        throw runtime_error("Agreement transaction id invalid\n");
    
    return(AgreementReferences(mypk,agreementtxid));
}

/*UniValue agreementupdatelog(const UniValue& params, bool fHelp, const CPubKey& mypk)
{
    uint256 agreementtxid;
	int64_t samplenum;
	std::string typestr;
	bool start_backwards;
    if ( fHelp || params.size() < 2 || params.size() > 3)
        throw runtime_error(
            "agreementupdatelog agreementtxid start_backwards ( num_samples )\n"
            "\nReturns array of agreement update transaction ids for the specified agreement transaction id.\n"
            + HelpRequiringPassphrase() +
            "\nArguments:\n"
            "1. agreementtxid (uint256, required) Valid agreement transaction id.\n"
            "2. start_backwards (numeric, required) Whether or not to sort ids from latest to oldest.\n"
            "3. num_samples (numeric, optional, default=0) Max amount of ids to retrieve. If 0, returns all ids.\n"
            "\nResult:\n"
            "\"result\"  (array of strings) Transaction ids of accepted agreement updates.\n"
            "\nExamples:\n"
            + HelpExampleCli("agreementupdatelog", "56b9bae388690d42fb13c7431d935acbda209bdafa239531549ab4de4b20802a 1 6")
            + HelpExampleRpc("agreementupdatelog", "56b9bae388690d42fb13c7431d935acbda209bdafa239531549ab4de4b20802a 1 6")
        );
    if ( ensure_CCrequirements(EVAL_AGREEMENTS) < 0 )
        throw runtime_error(CC_REQUIREMENTS_MSG);
    agreementtxid = Parseuint256((char *)params[0].get_str().c_str());
	typestr = params[1].get_str();
    if (STR_TOLOWER(typestr) == "1" || STR_TOLOWER(typestr) == "true")
        start_backwards = true;
    else if (STR_TOLOWER(typestr) == "0" || STR_TOLOWER(typestr) == "false")
        start_backwards = false;
    else 
        throw runtime_error("Incorrect sort type\n");
    if (params.size() >= 3)
		samplenum = atoll(params[2].get_str().c_str());
    else
        samplenum = 0;
    return(AgreementUpdateLog(agreementtxid, samplenum, start_backwards));
}

UniValue agreementinventory(const UniValue& params, bool fHelp, const CPubKey& mypk)
{
    CPubKey pubkey;
    if ( fHelp || params.size() > 1 )
        throw runtime_error(
            "agreementinventory ( \"pubkey\" )\n"
            "Returns three arrays (one for seller, client and arbitrator) of agreement transaction ids that the\n"
            "specified pubkey is a member of.\n"
            + HelpRequiringPassphrase() +
            "\nArguments:\n"
            "1. \"pubkey\" (string, optional, default=mypk) Pubkey to check for. If unset, pubkey used to launch\n"
            "the Komodo daemon is passed.\n"
            "\nResult:\n"
            "\"result\"  (json object) The agreement transaction ids that the pubkey is a member of.\n"
            "\nExamples:\n"
            + HelpExampleCli("agreementinventory", "\"0237b502085b2552ae4ac6b2b9faf8b215b34a540ecdb5e0b22d2d3b82219a0aea\"")
            + HelpExampleRpc("agreementinventory", "\"0237b502085b2552ae4ac6b2b9faf8b215b34a540ecdb5e0b22d2d3b82219a0aea\"")
        );
    if ( ensure_CCrequirements(EVAL_AGREEMENTS) < 0 )
        throw runtime_error(CC_REQUIREMENTS_MSG);
    if ( params.size() == 1 )
        pubkey = pubkey2pk(ParseHex(params[0].get_str().c_str()));
    else
		pubkey = mypk.IsValid() ? mypk : pubkey2pk(Mypubkey());
	return(AgreementInventory(pubkey));
}

UniValue agreementproposals(const UniValue& params, bool fHelp, const CPubKey& mypk)
{
    uint256 agreementtxid = zeroid;
	std::vector<unsigned char> pubkey;
    if ( fHelp || params.size() > 2 )
        throw runtime_error(
            "agreementproposals ( agreementtxid \"pubkey\" )\n"
            "\nReturns three arrays (one for seller, client and arbitrator) of agreement proposal transaction ids\n"
            "that the specified pubkey is referenced in.\n"
            + HelpRequiringPassphrase() +
            "\nArguments:\n"
            "1. agreementtxid (uint256, optional) Valid agreement transaction id. If set, will filter out proposals\n"
            "unrelated to this agreement.\n"
            "2. \"pubkey\" (string, optional, default=mypk) Pubkey to check for. If unset, pubkey used to launch\n"
            "the Komodo daemon is passed.\n"
            "\nResult:\n"
            "\"result\"  (json object) The agreement proposal transaction ids that the pubkey is referenced in.\n"
            "\nExamples:\n"
            + HelpExampleCli("agreementproposals", "56b9bae388690d42fb13c7431d935acbda209bdafa239531549ab4de4b20802a \"0237b502085b2552ae4ac6b2b9faf8b215b34a540ecdb5e0b22d2d3b82219a0aea\"")
            + HelpExampleRpc("agreementproposals", "56b9bae388690d42fb13c7431d935acbda209bdafa239531549ab4de4b20802a \"0237b502085b2552ae4ac6b2b9faf8b215b34a540ecdb5e0b22d2d3b82219a0aea\"")
        );
    if ( ensure_CCrequirements(EVAL_AGREEMENTS) < 0 )
        throw runtime_error(CC_REQUIREMENTS_MSG);
	if ( params.size() >= 1 )
        agreementtxid = Parseuint256((char *)params[0].get_str().c_str());
	if ( params.size() == 2 )
		pubkey = ParseHex(params[1].get_str().c_str());
    return(AgreementProposals(pubkey2pk(pubkey), agreementtxid));
}

UniValue agreementsubcontracts(const UniValue& params, bool fHelp, const CPubKey& mypk)
{
    uint256 agreementtxid;
    if ( fHelp || params.size() != 1 )
        throw runtime_error(
            "agreementsubcontracts agreementtxid\n"
            "\nReturns array of agreement transaction ids that reference the specified agreement transaction id\n"
            "as the master agreement.\n"
            + HelpRequiringPassphrase() +
            "\nArguments:\n"
            "1. agreementtxid (uint256, required) Valid agreement transaction id.\n"
            "\nResult:\n"
            "\"result\"  (array of strings) Transaction ids of subcontracts.\n"
            "\nExamples:\n"
            + HelpExampleCli("agreementsubcontracts", "56b9bae388690d42fb13c7431d935acbda209bdafa239531549ab4de4b20802a")
            + HelpExampleRpc("agreementsubcontracts", "56b9bae388690d42fb13c7431d935acbda209bdafa239531549ab4de4b20802a")
        );
    if ( ensure_CCrequirements(EVAL_AGREEMENTS) < 0 )
        throw runtime_error(CC_REQUIREMENTS_MSG);
    agreementtxid = Parseuint256((char *)params[0].get_str().c_str());
    return(AgreementSubcontracts(agreementtxid));
}

UniValue agreementsettlements(const UniValue& params, bool fHelp, const CPubKey& mypk)
{
    uint256 agreementtxid;
    if ( fHelp || params.size() != 2 )
        throw runtime_error(
            "agreementsettlements agreementtxid active_only\n"
            "\nReturns array of Pawnshop transaction ids that reference the specified agreement transaction id.\n"
            "Note: Unlike the other Agreements RPCs, this RPC will only return non-empty arrays\n"
            "for members of this agreement (not including the arbitrator).\n"
            + HelpRequiringPassphrase() +
            "\nArguments:\n"
            "1. agreementtxid (uint256, required) Valid agreement transaction id.\n"
            "2. active_only (numeric, required) if set, filters out closed Pawnshop instances.\n"
            "\nResult:\n"
            "\"result\"  (array of strings) Transaction ids of settlements.\n"
            "\nExamples:\n"
            + HelpExampleCli("agreementsettlements", "56b9bae388690d42fb13c7431d935acbda209bdafa239531549ab4de4b20802a 1")
            + HelpExampleRpc("agreementsettlements", "56b9bae388690d42fb13c7431d935acbda209bdafa239531549ab4de4b20802a 1")
        );
    if ( ensure_CCrequirements(EVAL_AGREEMENTS) < 0 || ensure_CCrequirements(EVAL_PAWNSHOP) < 0 )
        throw runtime_error(CC_REQUIREMENTS_MSG);
	std::string typestr;
	bool bActiveOnly;
    agreementtxid = Parseuint256((char *)params[0].get_str().c_str());
	typestr = params[1].get_str();
    if (STR_TOLOWER(typestr) == "1" || STR_TOLOWER(typestr) == "true")
        bActiveOnly = true;
    else if (STR_TOLOWER(typestr) == "0" || STR_TOLOWER(typestr) == "false")
        bActiveOnly = false;
    else 
        throw runtime_error("active_only flag invalid or empty\n");

    return(AgreementSettlements(mypk, agreementtxid, bActiveOnly));
}*/

UniValue agreementlist(const UniValue& params, bool fHelp, const CPubKey& mypk)
{
    UniValue result(UniValue::VARR);

	uint256 filtertxid;
    uint8_t flags;

    if (fHelp || params.size() > 2)
        throw runtime_error(
            "agreementlist [all|proposals|agreements][filtertxid]\n"
            );
    if (ensure_CCrequirements(EVAL_AGREEMENTS) < 0)
        throw runtime_error(CC_REQUIREMENTS_MSG);

    flags = ASF_ALL;
    if (params.size() >= 1)
    {
        if (STR_TOLOWER(params[0].get_str()) == "all")
            flags = ASF_ALL;
        else if (STR_TOLOWER(params[0].get_str()) == "proposals")
            flags = ASF_PROPOSALS;
        else if (STR_TOLOWER(params[0].get_str()) == "agreements")
            flags = ASF_AGREEMENTS;
        else
            throw runtime_error("Incorrect search keyword used\n");
    }
	
    if (params.size() == 2)
        filtertxid = Parseuint256((char *)params[1].get_str().c_str());

    return(AgreementList(mypk,flags,filtertxid));
}

static const CRPCCommand commands[] =
{ //  category              name                actor (function)        okSafeMode
  //  -------------- ------------------------  -----------------------  ----------
	// agreements
	{ "agreements",  "agreementcreate",  &agreementcreate,  true },
	{ "agreements",  "agreementstopproposal", &agreementstopproposal, true },
	{ "agreements",  "agreementaccept",  &agreementaccept,  true },
	{ "agreements",  "agreementupdate",  &agreementupdate,  true }, 
	{ "agreements",  "agreementclose",   &agreementclose,   true }, 
	{ "agreements",  "agreementdispute", &agreementdispute, true }, 
	{ "agreements",  "agreementresolve", &agreementresolve, true }, 
	{ "agreements",  "agreementunlock",  &agreementunlock,  true }, 
	{ "agreements",  "agreementaddress", &agreementaddress, true },
	{ "agreements",  "agreementinfo",	 &agreementinfo,	true },
	//{ "agreements",  "agreementupdatelog",	  &agreementupdatelog,    true },
	//{ "agreements",  "agreementinventory",	  &agreementinventory,    true },
	//{ "agreements",  "agreementproposals",	  &agreementproposals,    true },
	//{ "agreements",  "agreementsubcontracts", &agreementsubcontracts, true },
	//{ "agreements",  "agreementsettlements",  &agreementsettlements,  true },
	{ "agreements",  "agreementlist",    &agreementlist,    true },
};

void RegisterAgreementsRPCCommands(CRPCTable &tableRPC)
{
    for (unsigned int vcidx = 0; vcidx < ARRAYLEN(commands); vcidx++)
        tableRPC.appendCommand(commands[vcidx].name, &commands[vcidx]);
}
