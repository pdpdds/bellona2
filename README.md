# Bellona2 OS
서적 고급개발자들만이 알고 있던 OS 제작의 원리(2004년, 가남사)에서  
소개한 32비트 x86 Bellona2 OS 프로젝트의 소스코드입니다.  
최신 컴파일러 및 에뮬레이터에서 Bellona2 OS가 정상 동작하도록 작업했습니다.

## 빌드

아래 링크의 문서를 참조한다.  
[Bellona2 OS ](https://wikidocs.net/168662)

프로젝트는 비쥬얼 스튜디오 2022로 빌드가능하며
전체 자동화 빌드 액셜플로우는 액션탭에서 확인한다.

## 실행
현재 VMWARE, VirtualBox 에뮬레이터에서 커널 실행이 가능하다.

* 액션탭에서 bellona2를 다운받아 압축을 푼다.
* 릴리즈탭에서 hdd_image.rar를 다운받아 압축을 풀고 내용물을 bellona2 폴더에 복사한다.
* img2vmdk.bat 배치파일을 실행해서 VMWARE, VirtualBox용 FAT32 하드디스크 파일 hdd.vmdk를 생성한다.
* Bellona2.img 파일은 플로피 디스크로 마운트하고 hdd.vmdk는 IDE 하드 디스크로 가상 머신에 마운트한다.
* 가상머신으로 커널을 실행한 후 콘솔 입력창이 나오면 startg 111을 입력해서 GUI 모드를 가동한다.







