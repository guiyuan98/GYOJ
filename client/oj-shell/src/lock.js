const form = document.querySelector("#form");
const password = document.querySelector("#password");
const reason = document.querySelector("#reason");
const message = document.querySelector("#message");

window.gyoj.onLockReason((text) => {
  reason.textContent = text || "检测到违反考试客户端限制的行为。";
  password.value = "";
  message.textContent = "";
  setTimeout(() => password.focus(), 50);
});

form.addEventListener("submit", async (event) => {
  event.preventDefault();
  const result = await window.gyoj.unlockExam(password.value);
  if (!result.ok) {
    message.textContent = result.message || "解锁失败";
    password.select();
  }
});
