import { Meteor } from 'meteor/meteor';
import { Mongo } from 'meteor/mongo';
import { get_collection } from '../lib/collection_manager.js'
import '../lib/routes.js';

Meteor.startup(() => {
  //var TestVar = Mongo.Collection.getAll();
});

Meteor.methods({
  'checkcol'({sta,chan}){
    //console.log(sta);
    //console.log(chan);
    //console.log(get_collection(sta).find({'channel': chan}).count());
    if( get_collection(sta).find({'channel': chan}).count() !== 0 ) {
      //console.log("Collection not empty");
      return;
    }
    else{
      // Remove from the collection manager
      //console.log("Collection empty");
      throw new Meteor.Error(5, 'Station/Channel not found', 'Collection empty');
    }
  }
});
