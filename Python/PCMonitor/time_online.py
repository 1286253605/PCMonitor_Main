# -*- coding: utf-8 -*-

import os
import time
import requests


def getNetTime():
    try:
        head = {'User-Agent': 'Mozilla/5.0'}
        url = r'http://time1909.beijing-time.org/time.asp'
        # requests get
        r = requests.get(url=url, headers=head)
        # 检查返回码
        if r.status_code == 200:
            # 得到返回报文信息
            result = r.text
            print('r.text:', r.text)
            # 通过;分割文本；
            data = result.split(";")

            # print('data:',data)
            # ======================================================
            # 以下是数据文本处理：切割；
            year = data[1].split('=')[1]  # year=2021
            month = data[2].split('=')[1]
            day = data[3].split('=')[1]
            # wday = data[4].split('=')[1]
            hrs = data[5].split('=')[1]
            minute = data[6].split('=')[1]
            sec = data[7].split('=')[1]
            # ======================================================
            timestr = "%s/%s/%s %s:%s:%s" % (year, month, day, hrs, minute, sec)
            print(timestr)
            # 将timestr转为时间戳格式；
            timestrp = time.mktime(time.strptime(timestr, "%Y/%m/%d %X"))
            # 返回时间戳；
            return (timestrp, timestr)
    except:
        return (-1)


if __name__ == '__main__':
    # print('timer:', getNetTime())
    print(getNetTime()[1])
