#!/usr/bin/perl -w

# This small example comes from `perldoc POE::Kernel'.

use strict;

use POE; # auto-includes POE::Kernel and POE::Session

POE::Session->create(
    inline_states => {
        _start => sub {
            $_[KERNEL]->yield("next")
        },
        next => sub {
            print "tick (sid=", $_[SENDER]->ID(), ",", $_[CALLER_STATE], ")...\n";
            $_[KERNEL]->delay(next => 1);
        },
    },
);

POE::Kernel->run();
exit;

