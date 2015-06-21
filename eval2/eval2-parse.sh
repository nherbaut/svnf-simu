 for i in {0..16}; 


#do ../build/svnf_simu --nGW=20 --nDw=300 --mat=0.1 --popBitrate=1Gbps --cpBitrate=1Gbps  --vcs=1000 --vcv=100000 --ns3::TcpSocket::SegmentSize=14000 --ns3::TcpSocket::InitialCwnd=2 --as=10000000 --popDelay=25ms --cpDelay=50ms  --transTime=1  --gwUp=0.01 --pVidCount=100 --pStart=20 --pMeanArrTime=0.05 --PeakVcs=1000 --PeakVcv=0.1 --countBeforeCache=$i --cpop=$i 2>&1  > /dev/null &
do cut -c33-43 ./server-loads$i.csv | paste -sd+ | bc

done;
