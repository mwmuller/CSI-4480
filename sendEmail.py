import smtplib, ssl


sender_email = "someemail@gmail.com
rec_email = "email@gmail.com"
message = """\
	Subject: HI There


	This is a message from Python."""


port = 465

password = "password"

context = ssl.create_default_context()

with smtplib.SMTP_SSL("smtp.gmail.com", port, context=context) as server:
	server.login(sender_email, password)
	server.sendmail(sender_email, rec_email, message)
