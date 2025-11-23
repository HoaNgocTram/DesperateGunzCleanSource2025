<?php
// Thiết lập mã hóa UTF-8 để hỗ trợ tiếng Việt
header('Content-Type: text/html; charset=UTF-8');

// Khởi tạo kết nối ODBC (thay đổi thông tin kết nối theo cấu hình của bạn)
$dsn = "Driver={SQL Server};Server=localhost;Database=GunzDB;"; // Thay YOUR_SERVER và YOUR_DATABASE
$username = "sa"; // Thay YOUR_USERNAME
$password = "YOUR_PASSWORD"; // Thay YOUR_PASSWORD

// Kết nối ODBC
$connect = odbc_connect($dsn, $username, $password);
if (!$connect) {
    die("Kết nối ODBC thất bại: " . odbc_errormsg());
}

// Truy vấn số lượng người chơi trực tuyến
$query = odbc_exec($connect, "SELECT CurrPlayer FROM ServerStatus");
if (odbc_fetch_row($query)) {
    $onlinePlayers = odbc_result($query, 1);
} else {
    $onlinePlayers = 0; // Giá trị mặc định nếu truy vấn thất bại
}

// Đóng kết nối ODBC
odbc_close($connect);
?>

<!DOCTYPE html>
<html lang="vi">
<head>
    <title>GunZ - Weblauncher</title>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <meta http-equiv="cache-control" content="max-age=0">
    <meta http-equiv="cache-control" content="no-cache">
    <meta http-equiv="expires" content="0">
    <meta http-equiv="expires" content="Tue, 01 Jan 1980 1:00:00 GMT">
    <meta http-equiv="pragma" content="no-cache">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <link rel="preconnect" href="https://fonts.googleapis.com">
    <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin="">
    <link href="https://fonts.googleapis.com/css2?family=Open+Sans:ital,wght@0,300..800;1,300..800&display=swap" rel="stylesheet">
    <style type="text/css">
        * {
            padding: 0;
            margin: 0;
            list-style-type: none;
            text-decoration: none;
            font-size: 12px;
            font-family: 'Open Sans', 'Gill Sans', 'Gill Sans MT', 'Trebuchet MS', sans-serif;
        }
        html {
            overflow: hidden;
        }
        body {
            background: #1a1717;
        }
        .body {
            width: 710px;
            height: 434px;
            margin: 0 auto;
            background: url('images/cover16.jpg');
        }
        .wrapper {
            width: 710px;
        }
        .nav {
            width: 100px;
            float: left;
        }
        .nav li {
            width: 100px;
            height: 30px;
            color: #58595a;
            margin-top: 5px;
            cursor: pointer;
            text-transform: uppercase;
            text-indent: 14px;
            font-size: 11px;
            line-height: 32px;
            font-weight: 700;
        }
        .nav li:first-child {
            margin-top: 15px;
        }
        .nav li.active {
            background: url('images/highlighted.png') no-repeat;
            color: #fff;
            font-weight: 800;
        }
        .section {
            float: left;
            width: 610px;
            height: 434px;
        }
        .section li.active {
            color: #fff;
        }
        .discord span, .event span, .players span {
            font-size: 11px;
            text-transform: uppercase;
            font-weight: 700;
        }
        .event {
            margin: 70px 0px 12px 16px;
        }
        .discord, .players {
            margin: 12px 0px 12px 16px;
        }
        .players span:first-child {
            color: #fff;
        }
        .players span:last-child {
            color: #00bd31;
        }
        .event span:first-child {
            color: #fff;
        }
        .event span:last-child {
            color: #d42222;
        }
        .event.hasevent span:first-child {
            color: #00bd31;
        }
        .event.hasevent span:last-child {
            color: #00bd31;
        }
        .discord span:first-child {
            color: #fff;
        }
        .header {
            height: 70px;
        }
        .header .title {
            width: 300px;
            float: left;
        }
        .header .title h1, .header .title h2 {
            margin-left: 15px;
        }
        .header .title h1 {
            color: #f6fdff;
            text-transform: uppercase;
            font-size: 16px;
            font-weight: 700;
            margin-top: 8px;
        }
        .header .title h2 {
            color: #adadad;
            font-size: 10px;
            text-transform: uppercase;
            font-weight: 300;
        }
        .header .title h2 span {
            color: #2899ff;
            font-weight: 700;
            font-size: 12px;
        }
        .header div:last-child {
            width: 300px;
            float: left;
            position: relative;
        }
        .header div:last-child span {
            font-size: 11px;
            text-transform: uppercase;
            color: #5a5a5a;
            font-weight: 700;
        }
        .header div:last-child span span {
            color: #b02a2a;
            display: inline-block;
        }
        .price {
            margin-top: 23px;
            margin-left: 5px;
            display: block;
            width: 85px;
            float: left;
        }
        .strike {
            position: relative;
            color: #5a5a5a !important;
        }
        .strike::before {
            position: absolute;
            content: '';
            left: 0;
            top: 50%;
            right: 0;
            border: 1px solid red;
            -webkit-transform: rotate(-5deg);
            -moz-transform: rotate(-5deg);
            -ms-transform: rotate(-5deg);
            -o-transform: rotate(-5deg);
            transform: rotate(-5deg);
        }
        .name {
            display: none;
            position: absolute;
            top: 4px;
            left: -1px;
            text-align: center;
            color: #fff !important;
            background: rgba(0,0,0,0.4);
            border-radius: 0px 10px 10px 10px;
            width: 210px;
            height: 49px;
            line-height: 52px;
            font-size: 10px !important;
            z-index: 999;
        }
        .name:hover, .price:hover + .name, .img:hover + .name {
            display: block;
        }
        .img {
            display: block;
            float: left;
            margin: 10px 0 0 10px;
            text-align: center;
            width: 110px;
        }
        .img img {
            max-height: 40px;
            max-width: 100px;
        }
        .header a {
            background: #292929;
            border-radius: 15px;
            border: none;
            color: #fff;
            padding: 5px 15px;
            font-size: 9px;
            text-transform: uppercase;
            margin-top: 19px;
            margin-left: 5px;
            display: inline-block;
        }
        .title {
            display: none;
        }
        .title.title1 {
            display: block;
        }
        .content {
            display: none;
        }
        .content.news1 {
            display: block;
        }
        .content p {
            color: #cccccc;
            font-size: 12px;
            margin: 26px 0 0 15px;
            line-height: 18px;
            width: 585px;
            max-height: 325px;
            overflow: hidden;
        }
        .content a {
            color: #2899ff;
            font-size: 12px;
        }
        a:link,
        a:visited,
        a:hover,
        a:active {
            color: #2899ff;
            font-weight: 700;
        }
        a.c5 {
            color: #fff;
            font-weight: 700;
        }
    </style>
</head>
<body>
    <div class="body">
        <div class="wrapper">
            <div class="nav">
                <ul>
                    <li class="active">tháng 10 2025</li>
                    <li>tháng 08 2025</li>
                    <li>tháng 05 2025</li>
                    <li>tháng 04 2025</li>
                    <li>tháng 08 2023</li>
                    <li>tháng 05 2022</li>
                </ul>
                <div class="event">
                    <span class="c7"></span><br>
                    <span class="c8"></span>
                </div>
                <div class="discord">
                    <span class="c9">THAM GIA</span><br>
                    <a class="c10" href="https://discord.gg/jp7EXwgZaX" target="_blank">DISCORD</a>
                </div>
                <div class="players">
                    <span class="c4"><?php echo htmlspecialchars($onlinePlayers, ENT_QUOTES, 'UTF-8'); ?> NGƯỜI CHƠI</span>
                    <span>ONLINE</span>
                </div>
            </div>
            <div class="section">
                <div class="header">
                    <div class="title title1"><h1>Gunz Update</h1> <h2><span>ngày 2 tháng 10</span></h2></div>
                    <div class="title title2"><h1>Gunz Bảo trì</h1> <h2><span>ngày 24 tháng 08</span></h2></div>
                    <div class="title title3"><h1>Gunz Update</h1> <h2><span>ngày 03 tháng 05</span></h2></div>
                    <div class="title title4"><h1>Gunz Update</h1> <h2><span>ngày 29 tháng 04</span></h2></div>
                    <div class="title title5"><h1>Gunz Website</h1> <h2><span>ngày 01 tháng 08</span></h2></div>
                    <div class="title title6"><h1>Gunz Open  </h1> <h2><span>ngày 01 tháng 05</span></h2></div>
                    <div>
                        <span class="price">
                            <span class="strike c1">300 Cash</span>
                            <span class="c2">210 Cash</span>
                        </span>
                        <span class="name">Tất cả item</span>
                        <span class="img">
                            <img class="c3" src="https://gunz.vn/launchergunz/item/shotgun.png">
                        </span>
                        <span class="name">Tất cả item</span>
                        <a class="c5" href="https://gunz.vn" target="_blank">Home Page</a>
                    </div>
                </div>
                <div class="content news1"><p>• Ra mắt loạt set đồ hoàn toàn mới với thiết kế hiện đại, chi tiết hơn.
                                            <br>• Các bộ trang phục được chăm chút từng đường nét, tạo cảm giác chân thực và sống động.
                                            <br>• Mansion, Station và Town đã được làm mới với hiệu ứng ánh sáng và texture chất lượng cao. 
                                            <br>• Map mới SkirmishHall.
                                            <br>• Các animation mới mượt mà, đem lại cảm giác chân thực từng pha di chuyển, né tránh và tấn công.
                                            <br>• Để anh em có thể trải nghiệm trọn vẹn bản update, GunZ VN gửi tặng Giftcode: <b>GUNZVN2025</b>.
                                            <br>• Thêm hệ thống Easy Anti Cheat</p></div>
                <div class="content news2"><p>• Nhằm nâng cao chất lượng dịch vụ và đảm bảo trải nghiệm tốt nhất cho Gunzer, Công ty SNS sẽ tiến hành nâng cấp và chuyển đổi hạ tầng Internet. Việc này sẽ dẫn đến một số thay đổi về địa chỉ IP của dịch vụ game Gunz The Duel.
                                            <br>• Chúng tôi hiểu rằng việc thay đổi IP có thể gây ra một số bất tiện tạm thời. Tuy nhiên, SNS cam kết sẽ nỗ lực hết sức để quá trình chuyển đổi diễn ra suôn sẻ nhất có thể. Việc chuyển đổi IP là bắt buộc do chúng tôi thay đổi hạ tầng để đảm bảo cho việc đồng bộ về sau.
                                            <br>• Sau thời gian nâng cấp và chuyển đổi hạ tầng Internet, chúng tôi vui mừng thông báo rằng máy chủ Gunz The Duel đã chính thức hoạt động trở lại.
                                            <br>• Để tham gia game, các Gunzer vui lòng Tải client mới tại: [<a class="c5" href="https://gunz.vn/gunz/downloads" target="_blank">Download</a>]
                                            <br>• Đối với những ai đã cài client game cũ trước đây: chỉ cần tải GunzLauncher mới và copy vào thư mục game cũ là có thể chơi bình thường, không cần tải lại toàn bộ.</p></div>
                <div class="content news3"><p>• Launcher được tái tổ chức cấu trúc mã nguồn. Được xây dựng trên cơ sở mã tốt hơn, giữ nguyên thiết kế truyền thống, tăng hiệu xuất tải files update có dung lượng lớn.
                                            <br>• Thay thế giao thức tải files từ đơn luồn sang đa luồng. Giúp tối ưu thời gian tải các bản update.
                                            <br>• Thay thế giao thức checksum files cũ từ CRC sang MD5. Tính toán chính xác dung lượng tệp cần tải, tránh nhầm lẫn tải files về bị lỗi.
                                            <br>• Thêm tính năng tự động sử dụng card đồ họa rời bao gồm cả RTX 5000 series, RX 9000 series mới.
                                            <br>• Thêm hiển thị Crit Dmg cho tính năng Float dmg text.
                                            <br>• Chỉnh sửa tính năng Float dmg elu cải thiện tính toán và hiển thị chuẩn lượng sát thương. Thêm mới hiển thị dmg shotgun cho Quest mode.</p></div>
                <div class="content news4"><p>• Sửa đổi mã gốc cũ giúp game nhận diện thêm Windows 10, 11. Thêm nhận diện Vendor ID của các hãng sản xuất phần cứng Intel, NVDIA, AMD.
                                            <br>• Cải thiện hiệu năng giúp game tương thích với các card đồ họa thế hệ mới.
                                            <br>• Mở lại tính năng ghi patchlog để dễ dàng kiểm tra gỡ lỗi.
                                            <br>• Thêm Weblauncher giúp người chơi dễ dàng nhận thông tin cập nhật mới.
                                            <br>• Thêm tính năng Float dmg text (hiển thị sát thương tại ví trí mục tiêu).</p></div>
                <div class="content news5"><p>• Thay đổi cấu trúc, mã nguồn, giao diện website game.
                                            <br>• Cập nhật phiên bản php3 lên php7. Bổ sung nhiều tính năng hỗ trợ quản lý tài khoản. Tăng tính bảo mật.
                                            <br>• Tính năng Hỗ trợ đa ngôn ngữ</p></div>
                <div class="content news6"><p>• Đánh dáu ngày đầu tiên Gunz VN đi vào hoạt động. Được tài trợ bởi Công ty Cổ Phần SNS Hà Vũ
                                                Địa chỉ trụ sở: Số 6 Chùa Vẽ, Phường Đông Hải 1, Quận Hải An, Thành Phố Hải Phòng, Việt Nam
                                            <br>• Giấy phép kinh doanh số: 0201758611 do Sở kế hoạch và Đầu tư cấp ngày 03/05/2018.
                                            <br>• Để biết rõ hơn về GUNZ VN. Chúng tôi đã trả lời một số câu hỏi thường gặp từ phía người chơi.
                                            <br>• <a href="https://gunz.vn/gunz/faqs" target="_blank">Ấn Vào Đây</a> để xem chi tiết.</p></div>
            </div>
        </div>
    </div>

    <script>
        // Vô hiệu hóa chuột phải
        document.addEventListener('contextmenu', function(event) {
            event.preventDefault();
            //alert('Chuột phải đã bị vô hiệu hóa!'); // Thông báo tùy chọn
        });

        var element = document.getElementsByTagName('li');
        for (var i = 0; i < element.length; i++) {
            let j = i;
            element[i].addEventListener('click', function(li) {
                var divs = document.getElementsByClassName('news');
                for (var i = 0; i < divs.length; i++) {
                    divs[i].style.display = 'none';
                }

                var lis = document.getElementsByTagName('li');
                for (var i = 0; i < lis.length; i++) {
                    lis[i].className = '';
                }
                li.target.className = 'active';

                var news = document.getElementsByClassName('content');
                for (var k = 0; k < news.length; k++) {
                    news[k].style.display = 'none';
                }
                document.getElementsByClassName('news' + (j + 1))[0].style.display = 'block';

                var title = document.getElementsByClassName('title');
                for (var k = 0; k < title.length; k++) {
                    title[k].style.display = 'none';
                }
                document.getElementsByClassName('title' + (j + 1))[0].style.display = 'block';
            }, false);
        }

        var stop = false;
        getData();
        function getData() {
            var req = new XMLHttpRequest();
            req.open('GET', 'launcher.php', true);
            req.addEventListener('readystatechange', function(e) {
                if (e.target.readyState == 4 && e.target.status == 200) {
                    stop = true;
                    handleData(JSON.parse(e.target.response));
                }
            });
            req.setRequestHeader('Content-Type', 'application/json;charset=UTF-8');
            req.send();
        }

        setInterval(function() {
            if (stop) {
                return;
            }
            getData();
        }, 5000);

        function handleData(data) {
            // Item of the day.
            document.getElementsByClassName('c1')[0].innerHTML = data['item']['Price'] + ' coins';
            document.getElementsByClassName('c2')[0].innerHTML = data['item']['NewPrice'] + ' coins';
            document.getElementsByClassName('c3')[0].src = 'https://gunz.vn/launchergunz/item/duelkatana.png';
            document.getElementsByClassName('c5')[0].href = 'https://fgunz.net/itemshop/donation?id=' + data['item']['ID'];
            document.getElementsByClassName('name')[0].innerHTML = data['item']['Name'];
            document.getElementsByClassName('name')[1].innerHTML = data['item']['Name'];

            // Player count.
            document.getElementsByClassName('c4')[0].innerHTML = data['pc'][0] + ' PLAYERS';

            // Event state.
            var hasEvent = data['event'];
            document.getElementsByClassName('c7')[0].innerHTML = hasEvent ? 'EVENT IN' : 'CURRENTLY';
            document.getElementsByClassName('c8')[0].innerHTML = hasEvent ? 'PROGRESS' : 'NO EVENT';
            if (hasEvent) {
                document.getElementsByClassName('event')[0].className += ' hasevent';
            }

            // News items date.
            var element = document.getElementsByTagName('li');
            for (var i = 0; i < element.length; i++) {
                if (!data['news'][i]) {
                    continue;
                }
                element[i].innerHTML = data['news'][i]['date'];
            }

            // News item content.
            var element = document.getElementsByClassName('content');
            for (var i = 0; i < element.length; i++) {
                if (!data['news'][i]) {
                    continue;
                }
                element[i].innerHTML = '<p>' + data['news'][i]['message'] + '</p>';
            }

            // News item title.
            var element = document.getElementsByClassName('title');
            for (var i = 0; i < element.length; i++) {
                if (!data['news'][i]) {
                    continue;
                }
                element[i].innerHTML = '<h1>' + data['news'][i]['title'] + '</h1><h2><span>' + data['news'][i]['date'] + '</span></h2>';
            }
        }
    </script>
</body>
</html>