// Copyright (c) 2018 The NavCoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NAVCOIN_GOVERNANCE_H
#define NAVCOIN_GOVERNANCE_H

#include "amount.h"
#include "script/script.h"
#include "serialize.h"
#include "tinyformat.h"
#include "univalue/include/univalue.h"
#include "uint256.h"

using namespace std;

class CTransaction;
class CCoinsViewCache;

extern std::vector<std::pair<std::string, bool>> vAddedProposalVotes;
extern std::vector<std::pair<std::string, bool>> vAddedPaymentRequestVotes;

namespace CGovernance {

class CProposal;
class CPaymentRequest;
class CConsultation;

typedef unsigned int flags;

static const flags NIL = 0x0;
static const flags ACCEPTED = 0x1;
static const flags REJECTED = 0x2;
static const flags EXPIRED = 0x3;
static const flags PENDING_FUNDS = 0x4;
static const flags PENDING_VOTING_PREQ = 0x5;
static const flags FINISHED = 0x6;

void SetScriptForCommunityFundContribution(CScript &script);
void SetScriptForProposalVote(CScript &script, uint256 proposalhash, int vote);
void SetScriptForPaymentRequestVote(CScript &script, uint256 prequest, int vote);
void SetScriptForConsultationApprovalVote(CScript &script, uint256 consultation, int vote);

bool FindProposal(string propstr, CGovernance::CProposal &proposal);
bool FindProposal(uint256 prophash, CGovernance::CProposal &proposal);
bool FindPaymentRequest(uint256 preqhash, CGovernance::CPaymentRequest &prequest);
bool FindPaymentRequest(string preqstr, CGovernance::CPaymentRequest &prequest);
bool FindConsultation(uint256 chash, CGovernance::CConsultation &consultation);
bool FindConsultation(string cstr, CGovernance::CConsultation &consultation);

bool VoteProposal(string strProp, int vote, bool &duplicate);
bool VoteProposal(uint256 proposalHash, int vote, bool &duplicate);
bool RemoveVoteProposal(string strProp);
bool RemoveVoteProposal(uint256 proposalHash);

bool VoteConsultation(string strProp, int vote, bool &duplicate);
bool VoteConsultation(uint256 proposalHash, int vote, bool &duplicate);
bool RemoveVoteConsultation(string strProp);
bool RemoveVoteConsultation(uint256 proposalHash);

bool VotePaymentRequest(string strProp, int vote, bool &duplicate);
bool VotePaymentRequest(uint256 proposalHash, int vote, bool &duplicate);
bool RemoveVotePaymentRequest(string strProp);
bool RemoveVotePaymentRequest(uint256 proposalHash);

bool IsValidPaymentRequest(CTransaction tx, CCoinsViewCache& coins, int nMaxVersion);
bool IsValidProposal(CTransaction tx, int nMaxVersion);
bool IsValidConsultation(CTransaction tx, int nMaxVersion);

class CPaymentRequest
{
public:
    static const int32_t CURRENT_VERSION=3;

    CAmount nAmount;
    flags fState;
    uint256 hash;
    uint256 proposalhash;
    uint256 txblockhash;
    uint256 blockhash;
    uint256 paymenthash;
    int nVotesYes;
    int nVotesNo;
    string strDZeel;
    int nVersion;
    unsigned int nVotingCycle;

    CPaymentRequest() { SetNull(); }

    void SetNull() {
        nAmount = 0;
        fState = NIL;
        nVotesYes = 0;
        nVotesNo = 0;
        hash = uint256();
        blockhash = uint256();
        txblockhash = uint256();
        proposalhash = uint256();
        paymenthash = uint256();
        strDZeel = "";
        nVersion = 0;
        nVotingCycle = 0;
    }

    bool IsNull() const {
        return (nAmount == 0 && fState == NIL && nVotesYes == 0 && nVotesNo == 0 && strDZeel == "");
    }

    std::string GetState() const {
        std::string sFlags = "pending";
        if(IsAccepted()) {
            sFlags = "accepted";
            if(fState != ACCEPTED)
                sFlags += " waiting for end of voting period";
        }
        if(IsRejected()) {
            sFlags = "rejected";
            if(fState != REJECTED)
                sFlags += " waiting for end of voting period";
        }
        if(IsExpired())
            sFlags = "expired";
        return sFlags;
    }

    std::string ToString() const {
        return strprintf("CPaymentRequest(hash=%s, nVersion=%d, nAmount=%f, fState=%s, nVotesYes=%u, nVotesNo=%u, nVotingCycle=%u, "
                         " proposalhash=%s, blockhash=%s, paymenthash=%s, strDZeel=%s)",
                         hash.ToString(), nVersion, (float)nAmount/COIN, GetState(), nVotesYes, nVotesNo,
                         nVotingCycle, proposalhash.ToString(), blockhash.ToString().substr(0,10),
                         paymenthash.ToString().substr(0,10), strDZeel);
    }

    void ToJson(UniValue& ret) const;

    bool IsAccepted() const;

    bool IsRejected() const;

    bool IsExpired() const;

    bool ExceededMaxVotingCycles() const;

    bool CanVote(CCoinsViewCache& coins) const;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        if(ser_action.ForRead())
        {
            READWRITE(nAmount);
            // Payment Requests with versioning are signalled by a negative amount followed by the real amount
            if(nAmount < 0)
            {
                READWRITE(nAmount);
                READWRITE(this->nVersion);
            }
            else
            {
                nVersion = 1;
            }
        }
        else
        {
            CAmount nSignalVersion = -1;
            READWRITE(nSignalVersion);
            READWRITE(nAmount);
            READWRITE(this->nVersion);
        }
        READWRITE(fState);
        READWRITE(nVotesYes);
        READWRITE(nVotesNo);
        READWRITE(hash);
        READWRITE(proposalhash);
        READWRITE(blockhash);
        READWRITE(paymenthash);
        READWRITE(strDZeel);
        READWRITE(txblockhash);

        // Version-based read/write
        if(nVersion >= 2)
           READWRITE(nVotingCycle);
    }

};

class CProposal
{
public:
    static const int32_t CURRENT_VERSION=3;

    CAmount nAmount;
    CAmount nFee;
    std::string Address;
    uint32_t nDeadline;
    flags fState;
    int nVotesYes;
    int nVotesNo;
    std::vector<uint256> vPayments;
    std::string strDZeel;
    uint256 hash;
    uint256 blockhash;
    uint256 txblockhash;
    int nVersion;
    unsigned int nVotingCycle;

    CProposal() { SetNull(); }

    void SetNull() {
        nAmount = 0;
        nFee = 0;
        Address = "";
        fState = NIL;
        nVotesYes = 0;
        nVotesNo = 0;
        nDeadline = 0;
        vPayments.clear();
        strDZeel = "";
        hash = uint256();
        blockhash = uint256();
        nVersion = 0;
        nVotingCycle = 0;
    }

    bool IsNull() const {
        return (nAmount == 0 && nFee == 0 && Address == "" && nVotesYes == 0 && fState == NIL
                && nVotesNo == 0 && nDeadline == 0 && strDZeel == "");
    }

    std::string ToString(CCoinsViewCache& coins, uint32_t currentTime = 0) const;
    std::string GetState(uint32_t currentTime) const;

    void ToJson(UniValue& ret, CCoinsViewCache& coins) const;

    bool IsAccepted() const;

    bool IsRejected() const;

    bool IsExpired(uint32_t currentTime) const;

    bool ExceededMaxVotingCycles() const;

    uint64_t getTimeTillExpired(uint32_t currentTime) const;

    bool CanVote() const;

    bool CanRequestPayments() const {
        return fState == ACCEPTED;
    }

    bool HasPendingPaymentRequests(CCoinsViewCache& coins) const;

    CAmount GetAvailable(CCoinsViewCache& coins, bool fIncludeRequests = false) const;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        if (ser_action.ForRead()) {
            const_cast<std::vector<uint256>*>(&vPayments)->clear();
        }

        READWRITE(nAmount);

        if(ser_action.ForRead())
        {
            READWRITE(nFee);
            // Proposals with versioning are signalled by a negative fee followed by the real fee
            if(nFee < 0)
            {
                READWRITE(nFee);
                READWRITE(this->nVersion);
            }
            else
            {
                nVersion = 1;
            }
        }
        else
        {
            CAmount nSignalVersion = -1;
            READWRITE(nSignalVersion);
            READWRITE(nFee);
            READWRITE(this->nVersion);
        }

        READWRITE(Address);
        READWRITE(nDeadline);
        READWRITE(fState);
        READWRITE(nVotesYes);
        READWRITE(nVotesNo);
        READWRITE(*const_cast<std::vector<uint256>*>(&vPayments));
        READWRITE(strDZeel);
        READWRITE(hash);
        READWRITE(blockhash);
        READWRITE(txblockhash);

        // Version-based read/write
        if(nVersion >= 2) {
           READWRITE(nVotingCycle);
        }

    }
};

}

#endif // NAVCOIN_GOVERNANCE_H