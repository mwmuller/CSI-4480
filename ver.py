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
count = 0
# Address used for SMTP MAIL FROM command  
fromAddress = 'corn1@bt.com'
def doQuery( conn ):
	global count
	cur = conn.cursor(buffered=True)
	conn.commit()
	cur.execute("Select ID, email, domain from Info where verified = 0;")
 #This query might not be needed. The impact would be minimal
	if cur.rowcount > 0 : #Check if any rows were returned
		for ID, email, domain in cur.fetchall() :
				verify(email, domain, ID, conn)
		count = 0
	else:
		count = count + 1
#this method will pull from the database every half second
def dbThread( conn ):
	global count
	qTime = .25
	multiplier = 0
	while 1:
		time.sleep(qTime)
		doQuery( conn )
		print("did a Query: %s" %(count,))
		multiplier = count / (10*(int(multiplier) + 1)
		if qTime <= 3 and multiplier >= 1:
			qTime = (1*int(multiplier) - 1) + .25

 


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


# Assume SMTP response 250 is success
	if code == 250:
		print('Success')
		editEntry(email, domain, ID, conn, True)
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
