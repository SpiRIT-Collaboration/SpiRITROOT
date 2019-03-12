FISHTANKUSER=$2
DESTINATION=$1

# test and see if the designated directory exist
# by uploading a small test file first
TEMP=$(mktemp)
echo "Testing to see if connection can be estisblished..."
echo "test" > $TEMP
scp -q -o ConnectTimeout=10 -o ProxyCommand="ssh -q -W %h:%p $FISHTANKUSER@nsclgw1.nscl.msu.edu" $TEMP $FISHTANKUSER@fishtank:$DESTINATION
if [ $? -eq 0 ]; 
then
     echo "Connection test passed!"
     rm -f $TEMP
     ssh -o ProxyCommand="ssh -W %h:%p $FISHTANKUSER@nsclgw1.nscl.msu.edu" $FISHTANKUSER@fishtank "rm -f ${DESTINATION}/${TEMP}"
else
    echo "Test file failed to upload! Please check if the connection is correct! Abort!"
    echo "It is possible that you have not saved the authentication ssh keys for the server"
    echo "If so, please follow the following steps so we can scp to fishtank without password:"
    echo "1. SSH to nsclgw1 and enter 'ssh-keygen -t rsa'"
    echo "2. In nsclgw1, enter 'cat .ssh/id_rsa.pub | ssh FISHTANKUSER@fishtank 'cat >> .ssh/authorized_keys'"
    echo "3. SSH to dev-intel1X and enter 'ssh-keygen -t rsa'"
    echo "4. In dev-intel1X, enter 'cat .ssh/id_rsa.pub | ssh FISHTANKUSER@nsclgw1.nscl.msu.edu 'cat >> .ssh/authorized_keys'"
    echo "After that, check and see if files can be uploaded without passwords"
    echo "If that's not the problem, please check and see if there are spelling mistakes in the user and directory name"
    rm -f $TEMP
    exit 1
fi
