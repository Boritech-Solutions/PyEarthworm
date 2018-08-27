// Server entry point, imports all server code
import '/imports/startup/server';
import '/imports/startup/both';
import { getCollection } from '/imports/api/collections_manager/collections_manager.js' ;

Meteor.methods({
  'checkcol'({sta}){
    //console.log(sta);
    //console.log(getCollection(sta).find().count());
    if( getCollection(sta).find().count() !== 0 ) {
      console.log("Collection not empty");
      return;
    }
    else{
      // Remove from the collection manager
      console.log("Collection empty");
      throw new Meteor.Error(5, 'Station not found', 'Collection empty');
    }
  }
});