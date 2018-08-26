// Server entry point, imports all server code
import '/imports/startup/server';
import '/imports/startup/both';
import { get_collection } from '/imports/startup/both/collections_manager.js';

Meteor.methods({
  'checkcol'({sta}){
    //console.log(sta);
    //console.log(get_collection(sta).find().count());
    if( get_collection(sta).find().count() !== 0 ) {
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