# Bellona2 OS
서적 고급개발자들만이 알고 있던 OS 제작의 원리(2004년, 가남사)에서  
소개한 32비트 x86 Bellona2 OS 프로젝트의 소스코드입니다.  
최신 컴파일러 및 에뮬레이터에서 Bellona2 OS가 정상 동작하도록 작업했습니다.

[Bellona2 공식 홈페이지](http://www.bellona2.com)  


[![Bellona2 실행영상](http://img.youtube.com/vi/kg66S-3ia74/0.jpg)](https://youtu.be/kg66S-3ia74)

## 빌드

아래 링크의 문서를 참조한다.  
[Bellona2 OS 가이드 ](https://wikidocs.net/168662)

프로젝트는 비쥬얼 스튜디오 2022로 빌드가능하며
전체 자동화 빌드 액션플로우는 액션탭에서 확인한다.

## 실행
구동에 어려움이 있다면 이슈란에 글을 남겨 주세요.

### GRUB 부팅
* 비쥬얼 스튜디오 2022 또는 상위버전을 설치한다.
* 프로젝트를 다운받는다.
* 윈도우즈용 QEMU 최신버전을 다운받고 설치한다. 전역경로에서 실행되는지 확인한다.
* 액션탭에서 bellona2를 다운받아 압축을 압축을 풀고 내용물을 프로젝트의 disk 폴더에 복사한다.
* 릴리즈탭에서 hdd_image.rar를 다운받아 압축을 풀고 내용물을 프로젝트의 disk 폴더에 복사한다.
* kenel\bellona2.sln을 실행한 다음 시작 프로젝트를 Multiboot 프로젝트로 설정하고 실행한다.
* 커널을 실행한 후 콘솔 입력창이 나오면 startg 111을 입력해서 GUI 모드를 가동한다.

## 플로피 부팅
현재 VMWARE, VirtualBox 에뮬레이터에서 커널 실행이 가능하다.
PCEM에서도 실행 가능하며 실기에서는 베사모드를 지원하는 컴퓨터에서 실행 가능하다.

* 액션탭에서 bellona2를 다운받아 압축을 푼다.
* 릴리즈탭에서 hdd_image.rar를 다운받아 압축을 풀고 내용물을 bellona2 폴더에 복사한다.
* img2vmdk.bat 배치파일을 실행해서 VMWARE, VirtualBox용 FAT32 하드디스크 파일 hdd.vmdk를 생성한다.
* 가상머신에서 Bellona2.img 파일은 플로피 디스크로 마운트하고 hdd.vmdk는 IDE 하드 디스크로 가상 머신에 마운트한다.
* 커널을 실행한 후 콘솔 입력창이 나오면 startg 111을 입력해서 GUI 모드를 가동한다.

## WIP
* 시그널 전파 안됨(sigtest.exe)
* 응용앱 페이지 폴트 발생






