#!/usr/bin/python
#
# Functions for querying and controlling local Access Grid venue clients.
#
# Created November 2, 2010
# @author Andrew Ford

try:
    import agversion
    agversion.select(3)

    def GetVenueClients():
        from AccessGrid.VenueClient import GetVenueClientUrls
        return GetVenueClientUrls()

    def GetClient(clientURL):
        from AccessGrid.interfaces.VenueClient_client import VenueClientIW

        client = VenueClientIW(clientURL)
        try:
            if not client.IsValid():
                raise ValueError, "Error: venue client at %s not valid" % clientUrl
            else:
                return client
        except Exception, e:
            print e
            return None

    def GetExits(clientURL):
        client = GetClient(clientURL)
        if client == None:
            print "GetExits(): Error: getclient failed"
            return {}

        try:
            exits = client.GetConnections()
        except:
            print "Error: GetConnections() failed, maybe client is not in a venue?"
            return {}

        exitNames = dict([(exit.GetName(), exit.GetURI()) for exit in exits])
        return exitNames

    def EnterVenue(clientURL, venueURL):
        print "EnterVenue:", venueURL
        client = GetClient(clientURL)
        if client == None:
            print "EnterVenue(): Error: getclient failed"
            return

        client.EnterVenue(venueURL)

    def GetFormattedVenueStreams(clientURL, streamType):
        client = GetClient(clientURL)
        if client == None:
            print "GetFormattedVenueStreams(): Error: getclient failed"
            return {}

        streams = client.GetStreams()
        for stream in streams:
            # make sure location and networkLocations are the same thing
            assert(len(stream.networkLocations) == 1 and
                      repr(stream.location) == repr(stream.networkLocations[0]))

        # Lambda expressions are like anonymous functions and are equivalent to
        #   the below.
        #def criteria(s):
        #    return 'video' in [c.type for c in s.capability]]
        format = lambda s : "%s/%s"%(s.location.GetHost(), s.location.GetPort())
        key = lambda s : s.encryptionKey or '__NO_KEY__'
        criteria = lambda s : streamType in [c.type for c in s.capability]

        # if encryptionKey is None - ie, stream is not encrypted - send a dummy
        # value to signal to grav to not use encryption
        d = dict([(format(s), key(s)) for s in streams if criteria(s)])
        return d

    def GetCurrentVenueName(clientURL):
        # this is kind of a hack - grabs the name based on the name of the
        # streams in the venue, if there are any streams
        client = GetClient(clientURL)
        if client == None:
            print "GetCurrentVenueName(): Error: getclient failed"
            return ""

        streams = client.GetStreams()
        if len(streams) == 0:
            return ""
        else:
            return streams[0].name

    def VenueMoveTest():
        import random
        clientURL = GetVenueClients()[0]
        client = GetClient(clientURL)
        exits = GetExits(clientURL)
        for i in range(1000):
            j = random.randint(0, len(exits)-1)
            EnterVenue(clientURL, exits[exits.keys()[j]])
            exits = GetExits(clientURL)
            GetFormattedVenueStreams(clientURL, 'video')

except:
    import traceback
    traceback.print_exc()

#if __name__ == '__main__':
#    urls = GetVenueClients()
#    if len(urls) > 0:
#        exits = GetExits(urls[0])
#        print "Exits: "
#        import pprint
#        pprint.pprint(exits)
#        if len(exits) > 0:
#            print "exit key 0 type ", type(exits.keys()[0])
#            print "Entering first venue"
#            EnterVenue(urls[0], exits[exits.keys()[0]])
#        else:
#            print "No venue connections?"
#    else:
#        print "No Venue Clients found"

