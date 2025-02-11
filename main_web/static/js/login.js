document.getElementById('login-form').addEventListener('submit', (e) => {
    e.preventDefault(); // Ngăn form gửi request mặc định

    const email = document.getElementById('email').value;
    const password = document.getElementById('password').value;

    firebase.auth().signInWithEmailAndPassword(email, password)
        .then(userCredential => {
            // Lấy token từ Firebase
            userCredential.user.getIdToken().then(idToken => {
                // Gửi token qua server Django
                fetch('/dashboard/', {
                    method: 'GET',
                    headers: { 'Authorization': `Bearer ${idToken}` },
                }).then(response => {
                    if (response.redirected) {
                        window.location.href = response.url; // Chuyển hướng nếu đăng nhập thành công
                    } else {
                        console.error("Đăng nhập thất bại");
                    }
                });
            });
        })
        .catch(error => {
            console.error("Lỗi đăng nhập:", error.message);
            alert("Đăng nhập thất bại: " + error.message);
        });
});
