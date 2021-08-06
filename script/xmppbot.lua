function jidInitialised(account)
    print("Account initialised")
    print("From: "..jid(account))
    print("Instance: "..jin(account))
end
function jidConnected(account)
    print("Account connected")
    print("From: "..jid(account))
    print("Instance: "..jin(account))
end
function jidDisconnected(account)
    print("Account disconnected")
    print("From: "..jid(account))
    print("Instance: "..jin(account))
end
function messageReceived(from, to, message)
    print("Message received")
    print("From: "..jid(from))
    print("Instance: "..jin(from))
    print("Message: "..message)
end
function presenceReceived(from, presenceType, statusType, statusText)
    print("Presence received")
    print("From: "..jid(from))
    print("Instance: "..jin(from))
    print("Presence Type: "..presenceType)
    print("Status Type: "..statusType)
    print("Status Text: "..statusText)
end
