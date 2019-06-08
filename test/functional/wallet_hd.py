#!/usr/bin/env python3
# Copyright (c) 2016 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

from test_framework.test_framework import NavCoinTestFramework
from test_framework.util import *
import os
import shutil


class WalletHDTest(NavCoinTestFramework):

    def set_test_params(self):
        self.setup_clean_chain = True
        self.num_nodes = 2
        self.node_args = [['-usehd=0'], ['-usehd=1', '-keypool=0']]

    def run_test (self):
        tmpdir = self.options.tmpdir
        # Make sure we use hd, keep masterkeyid
        masterkeyid = self.nodes[1].getwalletinfo()['hdmasterkeyid']
        assert_equal(len(masterkeyid), 40)

        # Import a non-HD private key in the HD wallet
        non_hd_add = self.nodes[0].getnewaddress()
        self.nodes[1].importprivkey(self.nodes[0].dumpprivkey(non_hd_add))

        # This should be enough to keep the master key and the non-HD key
        self.nodes[1].backupwallet(tmpdir + "hd.bak")
        #self.nodes[1].dumpwallet(tmpdir + "hd.dump")

        # Derive some HD addresses and remember the last
        # Also send funds to each add
        slow_gen(self.nodes[0], 101)
        hd_add = None
        num_hd_adds = 50
        #an address has already been created on m/0/0/1, must offset
        for i in range(num_hd_adds):
            hd_add = self.nodes[1].getnewaddress()
            hd_info = self.nodes[1].validateaddress(hd_add)
            assert_equal(hd_info["hdkeypath"], "m/0'/0'/"+str(i+2)+"'")
            assert_equal(hd_info["hdmasterkeyid"], masterkeyid)
            self.nodes[0].sendtoaddress(hd_add, 1)
            slow_gen(self.nodes[0],1)
        self.nodes[0].sendtoaddress(non_hd_add, 1)
        slow_gen(self.nodes[0],1)

        self.sync_all()
        assert_equal(self.nodes[1].getbalance(), num_hd_adds + 1)

        print("Restore backup ...")
        self.stop_node(1)
        os.remove(self.options.tmpdir + "/node1/devnet/wallet.dat")
        shutil.copyfile(tmpdir + "hd.bak", tmpdir + "/node1/devnet/wallet.dat")
        self.start_node(1, self.node_args[1])
        #connect_nodes_bi(self.nodes, 0, 1)

        # Assert that derivation is deterministic
        hd_add_2 = None
        for _ in range(num_hd_adds):
            hd_add_2 = self.nodes[1].getnewaddress()
            hd_info_2 = self.nodes[1].validateaddress(hd_add_2)
            assert_equal(hd_info_2["hdkeypath"], "m/0'/0'/"+str(_+2)+"'")
            assert_equal(hd_info_2["hdmasterkeyid"], masterkeyid)
        assert_equal(hd_add, hd_add_2)
        # Needs rescan
        self.stop_node(1)
        self.start_node(1, self.node_args[1] + ['-rescan'])
        #connect_nodes_bi(self.nodes, 0, 1)
        assert_equal(self.nodes[1].getbalance(), num_hd_adds + 1)


if __name__ == '__main__':
    WalletHDTest().main ()
