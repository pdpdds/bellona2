# bellona2
32bit x86 bellona2 os source code

## Bellona2 세부 내용
아래 링크를 확인한다
[Bellona2 OS ](https://wikidocs.net/168662)


## 실행
현재 VMWARE, VirtualBox 에뮬레이터에서 커널 실행이 가능하다.

* 액션탭에서 bellona2를 다운받아 압축을 푼다.
* 릴리즈탭에서 hdd_image.rar를 다운받아 압축을 풀고 내용물을 bellona2 폴더에 복사한다.
* img2vmdk.bat 배치파일을 실행해서 VMWARE, VirtualBox용 FAT32 하드디스크 파일 hdd.vmdk를 생성한다.
* Bellona2.img 파일은 플로피 디스크로 마운트하고 hdd.vmdk는 IDE 하드 디스크로 가상 마운트에 마운트한다.
* 가상머신을 실행후 콘솔화면이 나오면 startg 111을 입력해서 GUI 모드를 가동한다.







