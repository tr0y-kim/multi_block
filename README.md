# multi_block
유해 사이트가 100만개이라고 가정을 하고 사용자가 접속하려는 사이트의 Host 이름이 100만개 리스트 안에 들어 갔을 때 패킷을 차단하는 프로그램을 작성 <br />

# Usage
./multiblock
코드 내부에 iptables 와 netfilter_queue 명령어를 저장해뒀으니 그냥 쓰시면 됩니다.

# 코드 설명
python 파일로 일단 백만개의 url들을 각 .com .kr등등의 파일들로 나누고 dot_list라는 폴더에 저장한다. <br />
file pointer를 오픈시켜서 <br />

`
cat /bin/cat 경로/dot_list/url에 붙는 단어 | grep "필터링 될 url"
`
<br />의 방식으로 검색하였다.

# 보안할 점
url에 접속할 때 정확하게 url 입력해줘야한다.
예를 들어 clock.zone 이라 안하고 clock.zone/ 이라해도 그대로 뚫리니 문제.
