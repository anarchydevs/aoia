#pragma once

#include "MFTreeView.h"
#include "DBManager.h"
#include <strstream>


class InventoryView;


class SqlTreeViewItemBase : public MFTreeViewItem
{
public:
   SqlTreeViewItemBase(InventoryView* pOwner);
   virtual ~SqlTreeViewItemBase();

   virtual void SetOwner(InventoryView* pOwner);
   virtual void SetLabel(std::tstring const& newLabel);
   virtual unsigned int AppendMenuCmd(HMENU hMenu, unsigned int firstID, WTL::CTreeItem item) const;
   virtual bool HandleMenuCmd(unsigned int commandID, WTL::CTreeItem item);

protected:
   InventoryView* m_pOwner;

   enum InternalCommand {
      CMD_EDIT = 1,
      CMD_DELETE,
   };

   mutable std::map<unsigned int, InternalCommand> m_commands;
};


class ContainerTreeViewItem : public SqlTreeViewItemBase
{
public:
   ContainerTreeViewItem(InventoryView* pOwner, unsigned int charid, unsigned int containerid, std::tstring const& constraints = std::tstring(), std::tstring const& label = std::tstring());
   virtual ~ContainerTreeViewItem();

   virtual void OnSelected();
   virtual bool CanEdit() const;
   virtual bool CanDelete() const { return true; }
   virtual std::tstring GetLabel() const;
   virtual std::vector<MFTreeViewItem*> GetChildren() const;
   virtual bool HasChildren() const;
   virtual unsigned int AppendMenuCmd(HMENU hMenu, unsigned int firstID, WTL::CTreeItem item) const;
   virtual bool HandleMenuCmd(unsigned int commandID, WTL::CTreeItem item);

private:
   unsigned int m_charid;
   unsigned int m_containerid;
   std::tstring m_label;
   std::tstring m_constraints;
};


class CharacterTreeViewItem : public SqlTreeViewItemBase
{
public:
   CharacterTreeViewItem(InventoryView* pOwner, unsigned int charid);
   virtual ~CharacterTreeViewItem();

   virtual void OnSelected();
   virtual bool CanEdit() const;
   virtual bool CanDelete() const { return true; }
   virtual std::tstring GetLabel() const;
   virtual void SetLabel(std::tstring const& newLabel);
   virtual std::vector<MFTreeViewItem*> GetChildren() const;
   virtual bool HasChildren() const;
   virtual unsigned int AppendMenuCmd(HMENU hMenu, unsigned int firstID, WTL::CTreeItem item) const;
   virtual bool HandleMenuCmd(unsigned int commandID, WTL::CTreeItem item);

private:
   unsigned int m_charid;
   std::tstring m_label;
};


class InventoryTreeRoot : public SqlTreeViewItemBase
{
public:
   InventoryTreeRoot(InventoryView* pOwner = NULL);
   virtual ~InventoryTreeRoot();

   virtual void OnSelected();
   virtual bool CanEdit() const;
   virtual bool CanDelete() const { return false; }
   virtual std::tstring GetLabel() const;
   virtual std::vector<MFTreeViewItem*> GetChildren() const;
   virtual bool HasChildren() const;
};
