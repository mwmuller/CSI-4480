import smtplib, ssl


sender_email = "mike4harbor1@hotmail.com"
rec_email = "ougoldvibrations@gmail.com"
message = """\
	Subject: HI There


	This is a message from Python."""


port = 465

password = "samsungs4"

context = ssl.create_default_context()

with smtplib.SMTP_SSL("smtp.gmail.com", port, context=context) as server:
	server.login(sender_email, password)
	server.sendmail(sender_email, rec_email, message)
