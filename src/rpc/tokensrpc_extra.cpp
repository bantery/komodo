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
#include "../cc/CCtokens.h"
#include "../cc/CCassets.h"

using namespace std;

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

static const CRPCCommand commands[] =
{ //  category              name                actor (function)        okSafeMode
  //  -------------- ------------------------  -----------------------  ----------
  // additional info RPCs for tokens
	{ "tokens",      "tokenowners",            &tokenowners,            true },
    { "tokens",      "tokeninventory",         &tokeninventory,         true },
};

void RegisterTokensExtraRPCCommands(CRPCTable &tableRPC)
{
    for (unsigned int vcidx = 0; vcidx < ARRAYLEN(commands); vcidx++)
        tableRPC.appendCommand(commands[vcidx].name, &commands[vcidx]);
}
