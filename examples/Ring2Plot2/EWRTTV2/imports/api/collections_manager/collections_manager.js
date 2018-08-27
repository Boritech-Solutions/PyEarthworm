import { Mongo } from 'meteor/mongo';
import { Meteor } from 'meteor/meteor';

const collections = {};

export const getCollection = name => {
    if (!collections[name]) {
      collections[name] = new Mongo.Collection(name);
      if (Meteor.isServer) {
        Meteor.publish(name, function tasksPublication() {
          return collections[name].find();
        });
      }
    }
    return collections[name];
}