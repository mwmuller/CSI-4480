import re
import smtplib
import threading
import time
from mysql.connector import MySQLConnection, Error
import threading, subprocess
import mysql.connector
import dns.resolver

hostname = '127.0.0.1'
username = 'root'
password = 'root'
dbname = 'deauth'
dbtable = 'Info'

# Address used for SMTP MAIL FROM command  
fromAddress = 'corn@bt.com'
def doQuery( conn ):
	cur = conn.cursor(buffered=True)
	conn.commit()
#Starting work on new join statment for cron
	cur.execute("Select ID, email, domain from Info where verified = 0;")
	print("done a cur")
 #This query might not be needed. The impact would be minimal
	if cur.rowcount > 0 : #Check if any rows were returned
		for ID, email, domain in cur.fetchall() :
				verify(email, domain, ID, conn)
		#for Addon_Pin, Addon_State, Addon_Dim, Addon_dimVal in cur.fetchall() :
		#GPIO(Addon_Pin, Addon_State, Addon_Dim, Addon_dimVal)

#this method will pull from the database every half second and update the GPIO pins
def dbThread( conn ):
	while 1:
		time.sleep(.5)
		doQuery( conn  )


def verify(email, domain, ID, conn):
	print(email)
	print(domain)
	# Simple Regex for syntax checking
	regex = '^[_a-z0-9-]+(\.[_a-z0-9-]+)*@[a-z0-9-]+(\.[a-z0-9-]+)*(\.[a-z]{2,})$'

	# Email address to verify
	inputAddress = email
	addressToVerify = str(inputAddress)
	print(addressToVerify)


	# Syntax check
	match = re.match(regex, addressToVerify)
	if match == None:
		
		print('Post back an error')

	# Get domain for DNS lookup
	#splitAddress = addressToVerify.split('@')
	print('Domain:', str(domain))

	# MX record lookup
	records = dns.resolver.query(domain, 'MX')
	mxRecord = records[0].exchange
	mxRecord = str(mxRecord)

	# SMTP lib setup (use debug level for full output)
	server = smtplib.SMTP()
	server.set_debuglevel(0)

	# SMTP Conversation
	server.connect(mxRecord)
	server.helo(server.local_hostname) ### server.local_hostname(Get local server hostname)
	server.mail(fromAddress)
	code, message = server.rcpt(str(email + '@' + domain))
	server.quit()

	#print(code)
	#print(message)

# Assume SMTP response 250 is success
	if code == 250:
		print('Success')
		editEntry(email, domain, ID, conn, True)
		#ver_to_Email_ID = addressToVerify
		#var_SUBJECT = "You've Won!"
		#var_EMAIL_BODY = "Give me all your personal info to win!"
		
		#email = Class_eMail()

		#email.send_Text_Mail(var_To_Email_ID, var_SUBJECT, var_EMAIL_BODY)

		#del email
	else:
		print('Bad')
		editEntry(email, domain, ID, conn, False)

def editEntry(email, domain, ID, conn, verified):
	cur = conn.cursor(buffered=True)
	#Starting work on new join statment for cron
	if verified == True:
		cur.execute("Update Info set verified = 1 where ID = %s;" %(ID,))
		print("updated")
	else:
		cur.execute("Delete from Info where ID = %s;" %(ID,))
		print("delete")
	conn.commit()
	
		

def connect():
	conn = "";
	try:
		conn = mysql.connector.connect(user=username, password=password, host=hostname, database=dbname)
	except mysql.connector.Error as err:
		print("No connect for you")
	cursor = conn.cursor() 
	#cursor.execute("insert into Info (ID, email, domain, password, verified) values ( 0,'mwmuller', 'oakland.edu', 'null', 0);") #Testing purposes
	print("connected")
	cursor.close()
	t = threading.Thread(target=dbThread, args=(conn,))
	t.start()

if __name__ == '__main__':
	connect()
