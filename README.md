# EMSL_2026f

[Lab 7 Fold](https://downgit.github.io/#/home?url=https://github.com/ZoeChengYu/EMSL_2026f/tree/main/Lab7)

110

# keyword
步驟 2：查詢分配到的 Major 編號並建立 /dev/demo 節點

``` bash
# 1. 查詢 demo 驅動的主編號
grep -w demo /proc/devices

# 2. 自動取得編號並建立節點 (或手動填入上方查到的數字，例如 238)
ledmajor=$(awk '$2=="demo" {print $1}' /proc/devices)
sudo mknod /dev/demo c "$ledmajor" 0
sudo chmod 666 /dev/demo

# 3. 確認節點已成功建立
ls -l /dev/demo
```
