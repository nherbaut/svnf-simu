for i in {1..16}
do
../build/svnf_simu --nGW=100 --nDw=1000 --mat=0.2 --popBitrate=1Gbps --cpBitrate=1Gbps  --vcs=1000 --vcv=1000 --ns3::TcpSocket::SegmentSize=14000 --ns3::TcpSocket::InitialCwnd=2 --as=10000000 --popDelay=25ms --cpDelay=50ms  --transTime=1  --gwUp=0.01 --pVidCount=200 --pStart=20 --pMeanArrTime=0.05 --PeakVcs=1000 --PeakVcv=0.1 --countBeforeCache=${i} --cpop=${i}  2>/dev/null 1>/dev/null &
done


