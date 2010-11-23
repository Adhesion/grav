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
        client = GetClient(clientURL)
        if client == None:
            print "EnterVenue(): Error: getclient failed"
            return

        client.EnterVenue(venueURL)

    def GetFormattedVenueStreams(clientURL, streamType):
        client = GetClient(clientURL)
        if client == None:
            print "EnterVenue(): Error: getclient failed"
            return []

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
        criteria = lambda s : streamType in [c.type for c in s.capability]

        return [format(s) for s in streams if criteria(s)]

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

