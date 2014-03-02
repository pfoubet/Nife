#! /bin/sh
if test -x nife
then
./nife benchm.nif
uname -a
echo "If your system is not present in the NEWS file, and if you have some time"
echo "Take a screen-shot and send your test at nife@seriane.fr"
echo "Thank you helping us in this challenge !"
else
echo "You are not in the src directory or you have not run 'make'"
fi
echo "The Nife Team."
