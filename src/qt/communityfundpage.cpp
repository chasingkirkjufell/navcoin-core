#include "communityfundpage.h"
#include "forms/ui_communityfundpage.h"
#include "communityfundpage.moc"

#include "main.h"
#include "txdb.h"
#include "wallet/wallet.h"
#include <string>
#include <iomanip>
#include <sstream>
#include <QAbstractScrollArea>

#include "communityfunddisplay.h"
#include "communityfunddisplaypaymentrequest.h"
#include "communityfundcreateproposaldialog.h"
#include "communityfundcreatepaymentrequestdialog.h"

CommunityFundPage::CommunityFundPage(const PlatformStyle *platformStyle, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CommunityFundPage),
    clientModel(0),
    walletModel(0),
    flag(CGovernance::NIL),
    viewing_proposals(true),
    viewing_voted(false),
    viewing_unvoted(false)
{
    ui->setupUi(this);

    // Hide horizontal scrollArea scroll bar
    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(ui->pushButtonProposals, SIGNAL(clicked()), this, SLOT(click_pushButtonProposals()));
    connect(ui->pushButtonPaymentRequests, SIGNAL(clicked()), this, SLOT(click_pushButtonPaymentRequests()));

    // Enable selection of pushButtonProposals by default
    ui->pushButtonProposals->setStyleSheet("QPushButton { background-color: #DBE0E8; }");
    ui->pushButtonPaymentRequests->setStyleSheet("QPushButton { background-color: #EDF0F3; }");

    // Connect push buttons to functions
    connect(ui->radioButtonAll, SIGNAL(clicked()), this, SLOT(click_radioButtonAll()));
    connect(ui->radioButtonYourVote, SIGNAL(clicked()), this, SLOT(click_radioButtonYourVote()));
    connect(ui->radioButtonPending, SIGNAL(clicked()), this, SLOT(click_radioButtonPending()));
    connect(ui->radioButtonAccepted, SIGNAL(clicked()), this, SLOT(click_radioButtonAccepted()));
    connect(ui->radioButtonRejected, SIGNAL(clicked()), this, SLOT(click_radioButtonRejected()));
    connect(ui->radioButtonExpired, SIGNAL(clicked()), this, SLOT(click_radioButtonExpired()));
    connect(ui->radioButtonNoVote, SIGNAL(clicked()), this, SLOT(click_radioButtonNoVote()));
    connect(ui->pushButtonCreateProposal, SIGNAL(clicked()), this , SLOT(click_pushButtonCreateProposal()));
    connect(ui->pushButtonCreatePaymentRequest, SIGNAL(clicked()), this, SLOT(click_pushButtonCreatePaymentRequest()));

    click_radioButtonPending();
    refresh(false, true);
}

void CommunityFundPage::setWalletModel(WalletModel *model)
{
    this->walletModel = model;
    flag = CGovernance::NIL;
    viewing_voted = false;
    viewing_unvoted = true;
    refresh(false, true);
    ui->radioButtonNoVote->setChecked(true);
}

void CommunityFundPage::refreshTab()
{
    refresh(ui->radioButtonAll->isChecked(), viewing_proposals);
}

void CommunityFundPage::deleteChildWidgets(QLayoutItem *item) {
  QLayout *layout = item->layout();
  if (layout)
  {
    int itemCount = ui->gridLayout->count();
    for (int index = 0; index < itemCount; index++)
    {
      deleteChildWidgets(ui->gridLayout->itemAt(index));
    }
  }
  delete item->widget();
}

void CommunityFundPage::reset()
{
    for (int index = ui->gridLayout->count() - 1; index >= 0; --index)
    {
      int row, column, rowSpan, columnSpan;
      ui->gridLayout->getItemPosition(index, &row, &column, &rowSpan, &columnSpan);
        QLayoutItem *item = ui->gridLayout->takeAt(index);
        deleteChildWidgets(item);
        delete item;
    }
}

void CommunityFundPage::append(QWidget* widget)
{
    int index = ui->gridLayout->count();
    int row, column, rowSpan, columnSpan;
    ui->gridLayout->getItemPosition(index, &row, &column, &rowSpan, &columnSpan);
    row = int(index/2);
    column = index%2;
    ui->gridLayout->addWidget(widget, row, column);
}

void CommunityFundPage::refresh(bool all, bool proposal)
{
    reset();

    // Format avaliable amount in the community fund
    string available;
    available = wallet->formatDisplayAmount(pindexBestHeader->nCFSupply);
    ui->labelAvailableAmount->setText(QString::fromStdString(available));

    // Format locked amount in the community fund
    string locked;
    locked = wallet->formatDisplayAmount(pindexBestHeader->nCFLocked);
    ui->labelLockedAmount->setText(QString::fromStdString(locked));

    {
        int64_t spent_nav = 0;
        std::vector<CGovernance::CPaymentRequest> vec;
        if(pblocktree->GetPaymentRequestIndex(vec))
        {
            BOOST_FOREACH(const CGovernance::CPaymentRequest& prequest, vec)
            {
                if(prequest.fState == CGovernance::ACCEPTED)
                {
                    spent_nav = spent_nav + prequest.nAmount;
                }
            }

            string spent;
            spent = wallet->formatDisplayAmount(spent_nav);
            ui->labelSpentAmount->setText(QString::fromStdString(spent));
        }
    }

    // Propulate proposal grid
    if (proposal) {
        std::vector<CGovernance::CProposal> vec;
        if(pblocktree->GetProposalIndex(vec))
        {
            BOOST_FOREACH(const CGovernance::CProposal proposal, vec) {
                // If wanting to view all proposals
                if (all) {
                    append(new CommunityFundDisplay(0, proposal));
                }
                else {
                    // If the filter is set to my vote, filter to only pending proposals which have been voted for
                    if (viewing_voted)
                    {
                        if (proposal.fState == CGovernance::NIL && proposal.GetState(pindexBestHeader->GetBlockTime()).find("expired") == string::npos)
                        {
                            auto it = std::find_if( vAddedProposalVotes.begin(), vAddedProposalVotes.end(),
                                                    [&proposal](const std::pair<std::string, int>& element){ return element.first == proposal.hash.ToString();} );
                            if (it != vAddedProposalVotes.end())
                            {
                                append(new CommunityFundDisplay(0, proposal));
                            } else
                            {
                                continue;
                            }
                        }
                        else
                        {
                            continue;
                        }
                    }
                    // If the filter is set to no vote, filter to only pending proposals which have been not been voted for yet
                    else if (viewing_unvoted)
                    {
                        if (proposal.fState == CGovernance::NIL && proposal.GetState(pindexBestHeader->GetBlockTime()).find("expired") == string::npos)
                        {
                            auto it = std::find_if( vAddedProposalVotes.begin(), vAddedProposalVotes.end(),
                                                    [&proposal](const std::pair<std::string, int>& element){ return element.first == proposal.hash.ToString();} );
                            if (it != vAddedProposalVotes.end())
                            {
                                continue;
                            } else
                            {
                                append(new CommunityFundDisplay(0, proposal));
                            }
                        }
                        else
                        {
                            continue;
                        }
                    }
                    else
                    {
                        // If the flag is expired, be sure to display proposals without the expired state if they have expired before the end of the voting cycle
                        if (proposal.fState != CGovernance::EXPIRED && proposal.GetState(pindexBestHeader->GetBlockTime()).find("expired") != string::npos && flag == CGovernance::EXPIRED)
                        {
                            append(new CommunityFundDisplay(0, proposal));
                        }
                        // If the proposal is accepted and waiting for funds or the end of the voting cycle, show in the accepted filter
                        if (flag == CGovernance::ACCEPTED && proposal.fState != CGovernance::ACCEPTED && proposal.GetState(pindexBestHeader->GetBlockTime()).find("accepted") != string::npos) {
                            append(new CommunityFundDisplay(0, proposal));
                        }
                        // If the proposal is rejected and waiting for funds or the end of the voting cycle, show in the rejected filter
                        if (flag == CGovernance::REJECTED && proposal.fState != CGovernance::REJECTED && proposal.GetState(pindexBestHeader->GetBlockTime()).find("rejected") != string::npos) {
                            append(new CommunityFundDisplay(0, proposal));
                        }
                        // Display proposals with the appropriate flag and have not expired before the voting cycle has ended
                        if (proposal.fState != flag || ((flag != CGovernance::EXPIRED && proposal.GetState(pindexBestHeader->GetBlockTime()).find("expired") != string::npos) ||
                                                        (flag != CGovernance::ACCEPTED && proposal.GetState(pindexBestHeader->GetBlockTime()).find("accepted") != string::npos) ||
                                                         (flag != CGovernance::REJECTED && proposal.GetState(pindexBestHeader->GetBlockTime()).find("rejected") != string::npos)))
                            continue;
                        append(new CommunityFundDisplay(0, proposal));
                    }
                }
            }
        }
    }
    else
    { //Payment request listings
        std::vector<CGovernance::CPaymentRequest> vec;
        if(pblocktree->GetPaymentRequestIndex(vec))
        {
            BOOST_FOREACH(const CGovernance::CPaymentRequest& prequest, vec) {
                // If wanting to view all prequests
                if (all)
                {
                    append(new CommunityFundDisplayPaymentRequest(0, prequest));
                }
                else
                {
                    // If the filter is set to my vote, filter to only pending prequests which have been voted for
                    if (viewing_voted)
                    {
                        if (prequest.fState == CGovernance::NIL && prequest.GetState().find("expired") == string::npos)
                        {
                            auto it = std::find_if( vAddedPaymentRequestVotes.begin(), vAddedPaymentRequestVotes.end(),
                                                    [&prequest](const std::pair<std::string, int>& element){ return element.first == prequest.hash.ToString();} );
                            if (it != vAddedPaymentRequestVotes.end())
                            {
                                append(new CommunityFundDisplayPaymentRequest(0, prequest));
                            } else
                            {
                                continue;
                            }
                        }
                        else
                        {
                            continue;
                        }
                    }
                    // If the filter is set to no vote, filter to only pending prequests which have not been voted for yet
                    else if (viewing_unvoted)
                    {
                        if (prequest.fState == CGovernance::NIL && prequest.GetState().find("expired") == string::npos)
                        {
                            auto it = std::find_if( vAddedPaymentRequestVotes.begin(), vAddedPaymentRequestVotes.end(),
                                                    [&prequest](const std::pair<std::string, int>& element){ return element.first == prequest.hash.ToString();} );
                            if (it != vAddedPaymentRequestVotes.end())
                            {
                                continue;
                            } else
                            {
                                append(new CommunityFundDisplayPaymentRequest(0, prequest));
                            }
                        }
                        else
                        {
                            continue;
                        }
                    }
                    else
                    {
                        // If the flag is expired, be sure to display prequests without the expired state if they have expired before the end of the voting cycle
                        if (prequest.fState != CGovernance::EXPIRED && prequest.GetState().find("expired") != string::npos && flag == CGovernance::EXPIRED)
                        {
                            append(new CommunityFundDisplayPaymentRequest(0, prequest));
                        }
                        // If the prequest is accepted and waiting for funds or the end of the voting cycle, show in the accepted filter
                        if (flag == CGovernance::ACCEPTED && prequest.fState != CGovernance::ACCEPTED && prequest.GetState().find("accepted") != string::npos) {
                            append(new CommunityFundDisplayPaymentRequest(0, prequest));
                        }
                        // If the prequest is rejected and waiting for funds or the end of the voting cycle, show in the rejected filter
                        if (flag == CGovernance::REJECTED && prequest.fState != CGovernance::REJECTED && prequest.GetState().find("rejected") != string::npos) {
                            append(new CommunityFundDisplayPaymentRequest(0, prequest));
                        }
                        // Display prequests with the appropriate flag and have not expired before the voting cycle has ended
                        if (prequest.fState != flag || ((flag != CGovernance::EXPIRED && prequest.GetState().find("expired") != string::npos) ||
                                                        (flag != CGovernance::ACCEPTED && prequest.GetState().find("accepted") != string::npos) ||
                                                        (flag != CGovernance::REJECTED && prequest.GetState().find("rejected") != string::npos)))
                            continue;
                        append(new CommunityFundDisplayPaymentRequest(0, prequest));
                    }
                }
            }
        }
    }
}

void CommunityFundPage::click_pushButtonProposals()
{
    QFont font = ui->pushButtonProposals->property("font").value<QFont>();

    ui->pushButtonProposals->setStyleSheet("QPushButton { background-color: #DBE0E8; }");
    ui->pushButtonPaymentRequests->setStyleSheet("QPushButton { background-color: #EDF0F3; }");

    QFont f(font.family(), font.pointSize(), QFont::Bold);
    ui->pushButtonProposals->setFont(f);
    ui->pushButtonPaymentRequests->setFont(f);

    viewing_proposals = true;
    refresh(ui->radioButtonAll->isChecked(), viewing_proposals);
}

void CommunityFundPage::click_pushButtonPaymentRequests()
{
    QFont font = ui->pushButtonPaymentRequests->property("font").value<QFont>();

    ui->pushButtonProposals->setStyleSheet("QPushButton { background-color: #EDF0F3; }");
    ui->pushButtonPaymentRequests->setStyleSheet("QPushButton { background-color: #DBE0E8; }");

    QFont f(font.family(), font.pointSize(), QFont::Bold);
    ui->pushButtonProposals->setFont(f);
    ui->pushButtonPaymentRequests->setFont(f);

    viewing_proposals = false;
    refresh(ui->radioButtonAll->isChecked(), viewing_proposals);
}

void CommunityFundPage::click_radioButtonAll()
{
    flag = CGovernance::NIL;
    viewing_voted = false;
    viewing_unvoted = false;
    refresh(true, viewing_proposals);
}

void CommunityFundPage::click_radioButtonYourVote()
{
    flag = CGovernance::NIL;
    viewing_voted = true;
    viewing_unvoted = false;
    refresh(false, viewing_proposals);
}

void CommunityFundPage::click_radioButtonPending()
{
    flag = CGovernance::NIL;
    viewing_voted = false;
    viewing_unvoted = false;
    refresh(false, viewing_proposals);
}

void CommunityFundPage::click_radioButtonAccepted()
{
    flag = CGovernance::ACCEPTED;
    viewing_voted = false;
    viewing_unvoted = false;
    refresh(false, viewing_proposals);
}

void CommunityFundPage::click_radioButtonRejected()
{
    flag = CGovernance::REJECTED;
    viewing_voted = false;
    viewing_unvoted = false;
    refresh(false, viewing_proposals);
}

void CommunityFundPage::click_radioButtonExpired()
{
    flag = CGovernance::EXPIRED;
    viewing_voted = false;
    viewing_unvoted = false;
    refresh(false, viewing_proposals);
}

void CommunityFundPage::click_pushButtonCreateProposal()
{
    CommunityFundCreateProposalDialog dlg(this);
    dlg.setModel(walletModel);
    dlg.exec();
    refresh(ui->radioButtonAll->isChecked(), viewing_proposals);
}

void CommunityFundPage::click_pushButtonCreatePaymentRequest()
{
    CommunityFundCreatePaymentRequestDialog dlg(this);
    dlg.setModel(walletModel);
    dlg.exec();
    refresh(ui->radioButtonAll->isChecked(), viewing_proposals);
}

void CommunityFundPage::click_radioButtonNoVote()
{
    flag = CGovernance::NIL;
    viewing_voted = false;
    viewing_unvoted = true;
    refresh(false, viewing_proposals);
}

CommunityFundPage::~CommunityFundPage()
{
    delete ui;
}
