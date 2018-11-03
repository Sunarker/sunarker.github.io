#!/usr/bin/python

'''
	By Jiangchao Yao.
	Needed packages:
		python-oauth2
	reference url:
		http://kb.cnblogs.com/page/189153/
		http://mkelsey.com/2011/07/03/Flickr-oAuth-Python-Example/
		http://mkelsey.com/2011/07/09/Flickr-Api-Grab-Photos-Example/
		https://www.flickr.com/services/api/auth.oauth.html
'''

def flickr_auth(apikey,apisecret,permission):
	import oauth2 as oauth
	import time
	import httplib2
	import urlparse
	import os,shutil
	
	# remove temporal files
	if os.path.exists('.Oflickr'):
		shutil.rmtree('.Oflickr')

	#-------------------------------------------------------------------------------
	print "Request token"

	url = "https://www.flickr.com/services/oauth/request_token"

	# Set the base oauth_* parameters along with any other parameters required
	# for the API call.
	params = {
		'oauth_timestamp': str(int(time.time())),
		'oauth_signature_method':"HMAC-SHA1",
		'oauth_version': "2.0",
		'oauth_callback': "https://www.flickr.com",
		'oauth_nonce': oauth.generate_nonce(),
		'oauth_consumer_key': apikey
	}

	# Setup the Consumer with the api_keys given by the provider
	consumer = oauth.Consumer(key=apikey, secret=apisecret)

	# Create our request. Change method, etc. accordingly.
	req = oauth.Request(method="GET", url=url, parameters=params)

	# Create the signature
	signature = oauth.SignatureMethod_HMAC_SHA1().sign(req,consumer,None)

	# Add the Signature to the request
	req['oauth_signature'] = signature

	# Make the request to get the oauth_token and the oauth_token_secret
	# I had to directly use the httplib2 here, instead of the oauth library.
	h = httplib2.Http(".Oflickr")
	resp, content = h.request(req.to_url(), "GET")

	#-------------------------------------------------------------------------------
	print "Authorize"

	authorize_url = "https://www.flickr.com/services/oauth/authorize"

	#parse the content
	request_token = dict(urlparse.parse_qsl(content))

	print "Request Token:"
	print "    - oauth_token        = %s" % request_token['oauth_token']
	print "    - oauth_token_secret = %s" % request_token['oauth_token_secret']
	print

	# Create the token object with returned oauth_token and oauth_token_secret
	token = oauth.Token(request_token['oauth_token'],
		request_token['oauth_token_secret'])

	# You need to authorize this app via your browser.
	print "Go to the following link in your browser:"
	print "%s?oauth_token=%s&perms=%s" % (authorize_url, request_token['oauth_token'],permission)
	print

	# Once you get the verified pin, input it
	accepted = 'n'
	while accepted.lower() == 'n':
		accepted = raw_input('Have you authorized me? (y/n) ')
	oauth_verifier = raw_input('What is the PIN? ')

	#set the oauth_verifier token
	token.set_verifier(oauth_verifier)

	#-------------------------------------------------------------------------------
	print "Access Token"

	# url to get access token
	access_token_url = "https://www.flickr.com/services/oauth/access_token"

	# Now you need to exchange your Request Token for an Access Token
	# Set the base oauth_* parameters along with any other parameters required
	# for the API call.
	access_token_parms = {
		'oauth_consumer_key': apikey,
		'oauth_nonce': oauth.generate_nonce(),
		'oauth_signature_method':"HMAC-SHA1",
		'oauth_timestamp': str(int(time.time())),
		'oauth_token':request_token['oauth_token'],
		'oauth_verifier' : oauth_verifier
	}

	#setup request
	req = oauth.Request(method="GET", url=access_token_url,
		parameters=access_token_parms)

	#create the signature
	signature = oauth.SignatureMethod_HMAC_SHA1().sign(req,consumer,token)

	# assign the signature to the request
	req['oauth_signature'] = signature

	#make the request
	h = httplib2.Http(".Oflickr")
	resp, content = h.request(req.to_url(), "GET")

	#parse the response
	access_token_resp = dict(urlparse.parse_qsl(content))

	#write out a file with the oauth_token and oauth_token_secret
	with open('token', 'w') as f:
		f.write(access_token_resp['oauth_token'] + '\n')
		f.write(access_token_resp['oauth_token_secret'])
	f.closed

	return access_token_resp

def testAPI(apikey,apisecret,token,token_secret):
	import oauth2 as oauth
	import time
	import httplib2
	import json

	url = "https://api.flickr.com/services/rest"
	params = {
		"format":"json",
		"nojsoncallback":1,
		"oauth_version":'2.0',
		'oauth_nonce': oauth.generate_nonce(),
		'oauth_timestamp': str(int(time.time())),
		'oauth_signature_method':"HMAC-SHA1",
		'oauth_consumer_key':apikey,
		'oauth_token':token
	}
	methods = {
		'method':'flickr.people.getGroups',
		'user_id':'124104546@N03'
	}

	params.update(methods)

	req = oauth.Request(method="GET", url=url, parameters=params)
	req['oauth_signature'] = oauth.SignatureMethod_HMAC_SHA1().sign(req,oauth.Consumer(key=apikey,secret=apisecret),oauth.Token(token,token_secret))

	h = httplib2.Http('.Oflickr')
	resp,content = h.request(req.to_url(),"GET")
	s = json.loads(content)
	print s	

if __name__ == "__main__":
	api_key = u""
	api_secret = u""
	#access_token_resp = flickr_auth(api_key,api_secret,'read')

	data = open('token')
	token = data.readline().rstrip('\n')
	secret = data.readline().rstrip('\n')
	testAPI(api_key,api_secret,token,secret)
	

